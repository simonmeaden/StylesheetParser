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
// class StartBraceNode;
// class EndBraceNode;

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
  WidgetModel();

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
  bool isValidPropertyValueForProperty(const QString& propertyname,
                                       const QString& valuename);
//  bool ifValidStylesheetValue(const QString& propertyname,
//                              const QString& valuename,
//                              StylesheetData* data);

  QStringList possibleWidgetsForSubControl(const QString& name);
  QStringList possibleSubControlsForWidget(const QString& widget);

  void addPseudoState(const QString& state, bool extraState = false);
  void removePseudoState(const QString& state);
  bool containsPseudoState(const QString& name);

  void addProperty(const QString& property, bool extraProperty = true);
  bool containsProperty(const QString& name);

  QStringList borderValues();

  QMultiMap<int, QString> fuzzySearch(const QString& name, QStringList list);
  QMultiMap<int, QString> fuzzySearchWidgets(const QString& name);
  QMultiMap<int, QString> fuzzySearchProperty(const QString& name);
  QMultiMap<int, QString> fuzzySearchPropertyValue(const QString& name,
                                                   const QString& value);
  QMultiMap<int, QString> fuzzySearchPseudoStates(const QString& name);
  QMultiMap<int, QString> fuzzySearchSubControl(const QString& name);

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

private:
  WidgetItem* m_root;
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

  bool checkPropertyValue(AttributeType propertyAttribute,
                          const QString& valuename);
  bool checkAlignment(const QString& value) const;
  bool checkAttachment(const QString& value) const;
  bool checkBackground(const QString& value) const;
  bool checkBool(const QString& value) const;
  bool checkBoolean(const QString& value) const;
  bool checkBorder(const QString& value) const;
  bool checkBorderImage(const QString& value) const;
  bool checkBorderStyle(const QString& value) const;
  bool checkBoxColors(const QString& value) const;
  bool checkBoxLengths(const QString& value) const;
  bool checkBrush(const QString& value) const;
  bool checkColor(const QString& value) const;
  bool checkFontStyle(const QString& value) const;
  bool checkFont(const QString& value) const;
  bool checkFontSize(const QString& value) const;
  bool checkFontWeight(const QString& value) const;
  bool checkGradient(const QString& value) const;
  bool checkIcon(const QString& value) const;
  bool checkLength(const QString& value) const;
  bool checkNumber(const QString& value) const;
  bool checkOutline(const QString& value) const;
  bool checkOrigin(const QString& value) const;
  bool checkOutlineStyle(const QString& value) const;
  bool checkOutlineColor(const QString& value) const;
  bool checkOutlineWidth(const QString& value) const;
  bool checkOutlineOffset(const QString& value) const;
  bool checkOutlineRadius(const QString& value) const;
  bool checkPaletteRole(const QString& value) const;
  bool checkRadius(const QString& value) const;
  bool checkRepeat(const QString& value) const;
  bool checkUrl(const QString& value) const;
  bool checkPosition(const QString& value) const;
  bool checkTextDecoration(const QString& value) const;
  //
  bool checkStylesheetEdit(const QString& value,
                           StylesheetData* data = nullptr) const;
  bool checkStylesheetEditBad(const QString& value,
                              StylesheetData* data = nullptr) const;
  bool checkStylesheetFontWeight(const QString& value,
                                 StylesheetData* data) const;
  QStringList eraseDuplicates(QStringList list);
};

class DataStore : public QObject
{
  Q_OBJECT
public:
  explicit DataStore(QObject* parent = nullptr);
  ~DataStore();

  void addWidget(const QString& widget, const QString& parent);
  QStringList widgets() { return m_widgetModel->widgets(); }
  void removeWidget(const QString& name);
  bool containsWidget(const QString& name);
  bool containsProperty(const QString& name);

  bool containsStylesheetProperty(const QString& name);
  bool containsPseudoState(const QString& name);
  bool containsSubControl(const QString& name);
  bool isValidSubControlForWidget(const QString& widget,
                                  const QString& subcontrol);
  bool checkSubControlForWidget(const QString& widgetName, const QString& name);

//  bool ifValidStylesheetValue(const QString& propertyname,
//                              const QString& valuename,
//                              StylesheetData* data);
  bool isValidPropertyValueForProperty(const QString& propertyname,
                                       const QString& value);
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

  //! Adds a new custom widget, with it's specialised names and values.
  bool addCustomWidget(const QString& name, const QString& parent)
  {
    return m_widgetModel->addCustomWidget(name, parent);
  }
  bool addCustomWidgetPseudoStates(const QString& name,
                                   const QStringList& states)
  {
    return m_widgetModel->addCustomWidgetPseudoStates(name, states);
  }
  bool addCustomWidgetSubControls(const QString& name,
                                  const QStringList& controls)
  {
    return m_widgetModel->addCustomWidgetSubControls(name, controls);
  }
  bool addCustomWidgetProperties(const QString& name,
                                 const QStringList& properties)
  {
    return m_widgetModel->addCustomWidgetProperties(name, properties);
  }
  bool addCustomWidgetPropertyValue(const QString& widget,
                                    const QString& property,
                                    const QString& value)
  {
    return m_widgetModel->addCustomWidgetPropertyValue(widget, property, value);
  }
  bool addCustomWidgetPropertyValues(const QString& widget,
                                     const QString& property,
                                     QStringList values)
  {
    return m_widgetModel->addCustomWidgetPropertyValues(
      widget, property, values);
  }
signals:
  void finished();

private:
  const QIcon m_invalidIcon, m_validIcon, m_addSemiColonIcon, m_addDColonIcon,
    m_addColonIcon, m_badSColonIcon, m_badColonIcon, m_badDColonIcon, m_noIcon,
    m_fuzzyIcon;
  QMutex m_mutex;
  QStringList m_attributeNames;
  QStringList m_possibleWidgets;
  QStringList m_StylesheetProperties;

  QMap<QTextCursor, Node*> m_nodes;
  int m_braceCount;
  bool m_manualMove, m_hasSuggestion;
  QTextCursor m_currentCursor;
  Node* m_currentNode;
  int m_maxSuggestionCount;

  bool isBraceCountZero();

  WidgetModel* m_widgetModel;
  void initialiseWidgetModel();
  //  QMap<QString, AttributeType> initialiseStylesheetMap();

  QMap<int, QString> fuzzyTestBrush(const QString& value);
};

#endif // DATASTORE_H
