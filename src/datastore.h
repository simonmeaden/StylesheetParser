/*
  Copyright 2020 Simon Meaden

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef DATASTORE_H
#define DATASTORE_H

#include <QAbstractItemModel>
#include <QFile>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QStack>
#include <QTextCursor>
#include <QtDebug>
#include <QtWidgets>

#include "common.h"

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "fts_fuzzy_match.h"

class StylesheetEditor;
class StylesheetData;
class Node;
class WidgetNode;
class DataStore;

class GradientCheck
{
  bool linear = false;
  bool radial = false;
  bool conical = false;

  virtual bool isRepeated() = 0;

public:
  enum Check
  {
    GoodName,
    Stop,
    BadName,
    Repeat,
  };
  enum FuzzyCheck
  {
    Good,
    Fuzzy,
    Bad,
  };

  GradientCheck() {}
  ~GradientCheck() {}

  virtual Check set(const QString& name) = 0;
};

class LinearCheck : public GradientCheck
{
  bool x1 = false, x2 = false, y1 = false, y2 = false;
  bool x1_r = false, x2_r = false, y1_r = false, y2_r = false;

  bool isRepeated() override { return (x1_r || x2_r || y1_r || y2_r); }

public:
  LinearCheck()
    : GradientCheck()
  {}

  Check set(const QString& name) override
  {
    if (name == "x1") {
      if (x1) {
        x1_r = true;
      } else {
        x1 = true;
      }
    } else if (name == "y1") {
      if (x2) {
        x2_r = true;
      } else {
        x2 = true;
      }
    } else if (name == "x2") {
      if (y1) {
        y1_r = true;
      } else {
        y1 = true;
      }
    } else if (name == "y2") {
      if (y2) {
        y2_r = true;
      } else {
        y2 = true;
      }
    } else if (name == "stop") {
      return Stop;
    } else {
      return BadName;
    }
    if (isRepeated()) {
      return Repeat;
    } else {
      return GoodName;
    }
  }
};

class RadialCheck : public GradientCheck
{
  bool cx = false, cy = false, cx_r = false, cy_r = false, fx = false,
       fx_r = false, fy = false, fy_r = false, radius = false, radius_r = false;

  bool isRepeated() override
  {
    return (cx_r || cy_r || fx_r || fy_r || radius_r);
  }

public:
  RadialCheck()
    : GradientCheck()
  {}

  Check set(const QString& name) override
  {
    if (name == "cx") {
      if (cx) {
        cx_r = true;
      } else {
        cx = true;
      }
    } else if (name == "cy") {
      if (cy) {
        cy_r = true;
      } else {
        cy = true;
      }
    } else if (name == "fx") {
      if (fx) {
        fx_r = true;
      } else {
        fx = true;
      }
    } else if (name == "fy") {
      if (fy) {
        fy_r = true;
      } else {
        fy = true;
      }
    } else if (name == "radius") {
      if (radius) {
        radius_r = true;
      } else {
        radius = true;
      }
    } else if (name == "stop") {
      return Stop;
    } else {
      return BadName;
    }
    if (isRepeated()) {
      return Repeat;
    } else {
      return GoodName;
    }
  }
};

class ConicalCheck : public GradientCheck
{
  bool cx = false, cy = false, cx_r = false, cy_r = false, angle = false,
       angle_r = false;

  bool isRepeated() override { return (cx_r || cy_r || angle_r); }

public:
  ConicalCheck()
    : GradientCheck()
  {}

  Check set(const QString& name) override
  {
    if (name == "cx") {
      if (cx) {
        cx_r = true;
      } else {
        cx = true;
      }
    } else if (name == "cy") {
      if (cy) {
        cy_r = true;
      } else {
        cy = true;
      }
    } else if (name == "angle") {
      if (angle) {
        angle_r = true;
      } else {
        angle = true;
      }
    } else if (name == "stop") {
      return Stop;
    } else {
      return BadName;
    }
    if (isRepeated()) {
      return Repeat;
    } else {
      return GoodName;
    }
  }
};

class Property
{
public:
  Property(const QString& name) { m_propertyname = name; }
  QString propertyName() const;
  void setPropertyName(const QString& propertyname);

  AttributeTypes attributes() const;
  void setAttributes(AttributeTypes attributes);

private:
  QString m_propertyname;
  AttributeTypes m_attributes;
};

class WidgetItem
{
public:
  WidgetItem(const QString& name, WidgetItem* parent);

  QString name();
  WidgetItem* parent();
  QList<WidgetItem*> children();
  void addChild(WidgetItem* child);
  void removeChild(const QString& name);
  bool hasChildren();
  bool isExtraWidget() const;
  void setExtraWidget(bool extraWidget);
  void addSubcontrol(const QString& control);
  bool removeSubcontrol(const QString& control);
  bool hasSubControl(const QString& name);
  QStringList subControls();

private:
  QString m_name;
  QStringList m_subcontrols;
  WidgetItem* m_parent = nullptr;
  QList<WidgetItem*> m_children;
  bool m_extraWidget = false;
};

class WidgetModel
{
public:
  WidgetModel(DataStore* datastore);

  void addWidget(const QString& parentName,
                 const QString& name,
                 bool extraWidget = false);
  //! \note only non-standard custom widgets that have been added can be
  //! removed.
  void removeWidget(const QString& name);
  bool hasWidget(const QString& widget);
  WidgetItem* widgetItem(const QString& name);

  void addSubControl(const QString& control, QStringList widgets);
  void removeSubControl(const QString& control);
  bool containsSubControl(const QString& control);
  bool checkSubControlForWidget(const QString& widget, const QString& control);

  bool isValidSubControlForWidget(const QString& widgetName,
                                  const QString& subcontrol);
  PropertyStatus* isValidPropertyValueForProperty(const QString& propertyname,
                                                  int& start,
                                                  const QString& valuename,
                                                  const QString& text);

  QStringList possibleWidgetsForSubControl(const QString& name);
  QStringList possibleSubControlsForWidget(const QString& widget);

  void addPseudoState(const QString& state, bool extraState = false);
  void removePseudoState(const QString& state);
  bool containsPseudoState(const QString& name);

  void addProperty(const QString& property, bool extraProperty = true);
  bool containsProperty(const QString& name);

  QStringList borderValues();

  QMultiMap<int, QString> fuzzySearch(const QString& name,
                                      QStringList list) const;
  QMultiMap<int, QString> fuzzySearchWidgets(const QString& name);
  QMultiMap<int, QString> fuzzySearchProperty(const QString& name);
  QMultiMap<int, QString> fuzzySearchPropertyValue(const QString& name,
                                                   const QString& value);
  QMultiMap<int, QString> fuzzySearchPseudoStates(const QString& name);
  QMultiMap<int, QString> fuzzySearchSubControl(const QString& name);
  QMultiMap<int, QString> fuzzySearchColorNames(const QString& name);

  AttributeType propertyValueAttribute(const QString& value);

  bool addCustomWidget(const QString& name, const QString& parent);
  QStringList widgets() { return m_widgets.keys(); }
  bool addCustomWidgetPseudoStates(const QString& widget, QStringList states);
  bool addCustomWidgetSubControls(const QString& widget, QStringList controls);
  bool addCustomWidgetProperties(const QString& widget, QStringList properties);
  bool addCustomWidgetPropertyValue(const QString& widget,
                                    const QString& property,
                                    QString value);
  bool addCustomWidgetPropertyValues(const QString& widget,
                                     const QString& property,
                                     QStringList values);

  QList<Theme*> customThemes() const;

  PropertyStatus* checkColorName(int start, const QString& value) const;
  PropertyStatus* checkColorHashValue(int start, const QString& value) const;
  PropertyStatus* checkColorRGB(int start, const QString& value) const;
  PropertyStatus* checkColorHS(int start, const QString& value) const;

  QString findNext(const QString& text,
                   int& pos,
                   bool showLineMarkers = false) const;
  void skipBlanks(const QString& text,
                  int& pos,
                  bool showLineMarkers = false) const;
  void stepBack(int& pos, const QString& block) const;

private:
  WidgetItem* m_root;
  DataStore* m_datastore;
  QMap<QString, WidgetItem*> m_widgets;
  QMap<QString, QList<WidgetItem*>> m_subControls;
  QMap<QString, AttributeType> m_attributes;
  QStringList m_pseudoStates;
  QList<bool> m_pseudoStatesExtra;
  QStringList m_properties;
  QList<bool> m_propertiesExtra;
  QStringList m_colors;
  QStringList m_paletteRoles;
  QStringList m_alignmentValues;
  QStringList m_gradient;
  QStringList m_attachment;
  QStringList m_borderStyle;
  QStringList m_borderImage;
  QStringList m_fontStyle;
  QStringList m_fontWeight;
  QStringList m_icon;
  QStringList m_origin;
  QStringList m_outlineStyle;
  QStringList m_outlineWidth;
  QStringList m_position;
  QStringList m_repeat;
  QStringList m_textDecoration;
  QString m_outlineColor;
  QStringList m_customNames;
  QMap<QString, QStringList> m_customPseudoStates;
  QMap<QString, QStringList> m_customSubControls;
  QMap<QString, QStringList> m_customProperties;
  QMap<QString, QMap<QString, QStringList>> m_customValues;
  QMap<QString, QMap<QString, AttributeTypes>> m_customAttributes;
  QStringList recurseWidgetsForNames(WidgetItem* item);
  QStringList addControls(int count, ...);

  void initPseudoStates();
  void initPaletteRoles();
  void initColorNames();
  void initProperties();
  void initSubControls();
  void initWidgetTree();
  void initAlignment();
  void initGradients();
  void initAttachments();
  void initBorderStyle();
  void initBorderImage();
  void initFontStyle();
  void initFontWeight();
  void initIcon();
  void initOrigin();
  void initOutlineStyle();
  void initOutlineColor();
  void initOutlineWidth();
  void initPosition();
  void initRepeat();
  void initTextDecoration();
  void initAttributeMap();

  PropertyStatus* checkPropertyValue(AttributeType propertyAttribute,
                                     int& start,
                                     const QString& valuename,
                                     const QString& text);
  PropertyStatus* checkAlignment(const QString& value, int start = -1) const;
  PropertyStatus* checkAttachment(const QString& value, int start = -1) const;
  PropertyStatus* checkBackground(const QString& value, int start = -1) const;
  PropertyStatus* checkBool(const QString& value, int start = -1) const;
  PropertyStatus* checkBoolean(const QString& value, int start = -1) const;
  PropertyStatus* checkBorder(const QString& value, int start = -1) const;
  PropertyStatus* checkBorderImage(const QString& value, int start = -1) const;
  PropertyStatus* checkBorderStyle(const QString& value, int start = -1) const;
  PropertyStatus* checkBoxColors(const QString& value,
                                 int& pos,
                                 const QString& text = QString()) const;
  PropertyStatus* checkBoxLengths(const QString& value,
                                  int& start,
                                  const QString& text = QString()) const;
  PropertyStatus* checkBrush(const QString& value, int start = -1) const;
  PropertyStatus* checkColor(const QString& value, int start = -1) const;
  PropertyStatus* checkFontStyle(const QString& value, int start = -1) const;
  PropertyStatus* checkFont(const QString& value, int start = -1) const;
  PropertyStatus* checkFontSize(const QString& value, int start = -1) const;
  PropertyStatus* checkFontWeight(const QString& value, int start = -1) const;
  PropertyStatus* checkGradient(const QString& value, int start = -1) const;
  GradientCheck::FuzzyCheck checkGradientColor(const QString& value,
                                               int start = -1) const;
  bool checkGradientNumber(const QString& value) const;
  PropertyStatus* checkIcon(const QString& value, int start = -1) const;
  PropertyStatus* checkLength(const QString& value, int start = -1) const;
  PropertyStatus* checkNumber(const QString& value, int start = -1) const;
  PropertyStatus* checkOutline(const QString& value, int start = -1) const;
  PropertyStatus* checkOrigin(const QString& value, int start = -1) const;
  PropertyStatus* checkOutlineStyle(const QString& value, int start = -1) const;
  PropertyStatus* checkOutlineColor(const QString& value, int start = -1) const;
  PropertyStatus* checkOutlineWidth(const QString& value, int start = -1) const;
  PropertyStatus* checkOutlineOffset(const QString& value,
                                     int start = -1) const;
  PropertyStatus* checkOutlineRadius(const QString& value,
                                     int start = -1) const;
  PropertyStatus* checkPaletteRole(const QString& value, int start = -1) const;
  PropertyStatus* checkRadius(const QString& value, int start = -1) const;
  PropertyStatus* checkRepeat(const QString& value, int start = -1) const;
  PropertyStatus* checkUrl(const QString& value, int start = -1) const;
  PropertyStatus* checkPosition(const QString& value, int start = -1) const;
  PropertyStatus* checkTextDecoration(const QString& value,
                                      int start = -1) const;

  QStringList eraseDuplicates(QStringList list);
  QPair<PropertyStatus*, int> calculateNumericalStatus(
    const QString& section,
    const QString& cleanValue,
    const QString& number,
    int start,
    int offset,
    QStringList parts) const;
  QPair<PropertyStatus*, int> calculateStopStatus(const QString& section,
                                                  const QString& cleanValue,
                                                  const QString& number,
                                                  const QString& color,
                                                  int start,
                                                  int offset,
                                                  QStringList parts) const;
  GradientCheck* getCorrectCheck(const QString& name) const;
};

class DataStore : public QObject
{
  Q_OBJECT
public:
  explicit DataStore(QWidget* parent = nullptr);
  ~DataStore();

  void setEditor(StylesheetEditor* editor);

  QTextCursor getCursorForPosition(int position);
  QRect getRectForText(int start, const QString& text);

  void addWidget(const QString& widget, const QString& parent);
  QStringList widgets() { return m_widgetModel->widgets(); }
  void removeWidget(const QString& name);
  bool containsWidget(const QString& name);
  bool containsProperty(const QString& name);

  //  bool containsStylesheetProperty(const QString& name);
  bool containsPseudoState(const QString& name);
  bool containsSubControl(const QString& name);
  bool isValidSubControlForWidget(const QString& widget,
                                  const QString& subcontrol);
  bool checkSubControlForWidget(const QString& widgetName, const QString& name);

  //  bool ifValidStylesheetValue(const QString& propertyname,
  //                              const QString& valuename,
  //                              StylesheetData* data);
  PropertyStatus* isValidPropertyValueForProperty(
    const QString& propertyname,
    int& start,
    const QString& valuename,
    const QString& text = QString());
  AttributeType propertyValueAttribute(const QString& value);

  //! Returns the names of all
  //! widgets for which this
  //! sub-control is valid.
  QStringList possibleWidgetsForSubControl(const QString& name);
  QStringList possibleSubControlsForWidget(const QString& widget);

  void addSubControl(const QString& control, const QString& widget);
  void addSubControl(const QString& control, QStringList& widgets);
  void removeSubControl(const QString& control);
  void addPseudoState(const QString& state);
  void removePseudoState(const QString& state);

  QMap<QTextCursor, Node*> nodes();
  void insertNode(QTextCursor cursor, Node* node);
  bool isNodesEmpty();
  void clearNodes();
  void setNodes(QMap<QTextCursor, Node*> nodes);

  int braceCount();
  void setBraceCount(int value);
  void incrementBraceCount();
  bool decrementBraceCount();

  int maxSuggestionCount();
  void setMaxSuggestionCount(int maxSuggestionCount);

  bool hasSuggestion();
  void setHasSuggestion(bool suggestion);

  bool isManualMove();
  void setManualMove(bool manualMove);

  Node* currentNode();
  void setCurrentWidget(WidgetNode* value);
  bool isCurrentWidget(WidgetNode* node);

  QTextCursor currentCursor();
  void setCurrentCursor(const QTextCursor& currentCursor);

  QIcon invalidIcon() const;
  QIcon validIcon() const;
  QIcon addDColonIcon() const;
  QIcon addSemiColonIcon() const;
  QIcon addColonIcon() const;
  QIcon badSemiColonIcon() const;
  QIcon badColonIcon() const;
  QIcon badDColonIcon() const;
  QIcon noIcon() const;
  QIcon fuzzyIcon() const;

  QMultiMap<int, QString> fuzzySearchWidgets(const QString& name);
  QMultiMap<int, QString> fuzzySearchProperty(const QString& name);
  QMultiMap<int, QString> fuzzySearchPropertyValue(const QString& name,
                                                   const QString& value);
  QMultiMap<int, QString> fuzzySearchPseudoStates(const QString& name);
  QMultiMap<int, QString> fuzzySearchSubControl(const QString& name);
  QMultiMap<int, QString> fuzzySearchColorNames(const QString& name);

  //! Adds a new custom widget, with it's specialised names and values.
  bool addCustomWidget(const QString& name, const QString& parent);
  bool addCustomWidgetPseudoStates(const QString& name,
                                   const QStringList& states);
  bool addCustomWidgetSubControls(const QString& name,
                                  const QStringList& controls);
  bool addCustomWidgetProperties(const QString& name,
                                 const QStringList& properties);
  bool addCustomWidgetPropertyValue(const QString& widget,
                                    const QString& property,
                                    const QString& value);
  bool addCustomWidgetPropertyValues(const QString& widget,
                                     const QString& property,
                                     QStringList values);
  QStringList fontSizes();

  static const QFont NORMALFONT;
  static const QFont LIGHTFONT;
  static const QFont BOLDFONT;
  static const QString URL_REGEX;

  QList<QString> customThemeNames() const;
  QMap<QString, Theme*> customThemes() const;

  void setConfigurationLocation();
  QString configDir() const;
  QString configFile() const;
  QString qtTheme() const;
  QString qtThemeName() const;

  bool loadXmlTheme(const QString&);
  bool saveXmlScheme(const QString& name);
  bool loadConfig(const QString& filename = QString());
  bool saveConfig(const QString& filename = QString());

  QString currentTheme() const;
  QString currentThemeName() const;

  QString findNext(const QString& text,
                   int& pos,
                   bool showLineMarkers = false) const;
  void skipBlanks(const QString& text,
                  int& pos,
                  bool showLineMarkers = false) const;
  void stepBack(int& pos, const QString& block);

signals:
  void finished();

private:
  QWidget* m_parent;
  StylesheetEditor* m_editor;
  const QIcon m_invalidIcon, m_validIcon, m_addSemiColonIcon, m_addDColonIcon,
    m_addColonIcon, m_badSColonIcon, m_badColonIcon, m_badDColonIcon, m_noIcon,
    m_fuzzyIcon;
  QMutex m_mutex;
  QStringList m_attributeNames;
  QStringList m_possibleWidgets;
  //  QStringList m_StylesheetProperties;
  QMap<QString, Theme*> m_customThemes;
  QString m_configDir;
  QString m_configFile;
  QString m_qtConfigDir;
  QString m_qtConfigFile;
  QString m_qtTheme;
  QString m_qtThemeName;
  QString m_currentTheme;
  QString m_currentThemeName;

  QMap<QTextCursor, Node*> m_nodes;
  int m_braceCount;
  bool m_manualMove, m_hasSuggestion;
  QTextCursor m_currentCursor;
  Node* m_currentNode;
  int m_maxSuggestionCount;

  void readStandardThemes();
  void readCustomThemes();
  bool isBraceCountZero();

  WidgetModel* m_widgetModel;
  void initialiseWidgetModel();
  //  QMap<QString, AttributeType> initialiseStylesheetMap();

  QMap<int, QString> fuzzyTestBrush(const QString& value);
  void readStyleFile(QFileInfo info);
  void readQtCreatorTheme();
};

#endif // DATASTORE_H
