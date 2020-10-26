/*
  Copyright 2020 Simon Meaden

  Permission is hereby granted, free of charge, to any person obtaining a copy of this
  software and associated documentation files (the "Software"), to deal in the Software
  without restriction, including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software, and to permit
                                                                         persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef DATASTORE_H
#define DATASTORE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QFile>

#define FTS_FUZZY_MATCH_IMPLEMENTATION
#include "fts_fuzzy_match.h"

class StylesheetEdit;
class StylesheetData;

class DataStore : public QObject
{
  Q_OBJECT
public:
  enum AttributeType
  {
    NoAttributeValue,
    Alignment,
    Attachment,
    Background,
    Bool, //!< This is true/false value
    Boolean, //!< This is 0/1 value
    Border,
    BorderImage,
    BorderStyle,
    BoxColors,
    BoxLengths,
    Brush,
    Color,
    Font,
    FontSize,
    FontStyle,
    FontWeight,
    Gradient,
    Icon,
    Length,
    Number,
    Origin,
    PaletteRole,
    Radius,
    Repeat,
    Url,
    String,
    Outline,
    OutlineStyle,
    OutlineRadius,
    Position,
    TextDecoration,
    List,
    // below here are specific to StylesheetEdit
    StylesheetEditGood,
    StylesheetEditBad,
  };

  explicit DataStore(QObject* parent);
  ~DataStore();

  //  QStringList widgets() const;

  void addWidget(const QString& widget);
  void removeWidget(const QString& widget);
  bool containsWidget(const QString& name);
  QMap<int, QString> fuzzySearchWidgets(const QString& name);
  bool containsProperty(const QString& name);
  QMap<int, QString> fuzzySearchProperty(const QString& name);
  QMap<int, QString> fuzzySearchPropertyValue(const QString& name, const QString &value);
  bool containsStylesheetProperty(const QString& name);
  bool containsPseudoState(const QString& name);
  QMap<int, QString> fuzzySearchPseudoStates(const QString& name);
  bool containsSubControl(const QString& name);
  QMap<int, QString> fuzzySearchSubControl(const QString& name);

  bool getIfValidStylesheetValue(const QString& propertyname, const QString& valuename,
                                 StylesheetData* data);
  bool isValidPropertyValueForProperty(const QString& propertyname, const QString& value);
  //  QList<bool> isValidPropertyValues(const QString& name, const QStringList& values);
  AttributeType propertyValueAttribute(const QString& value);
//  QMap<int, QString> propertyValueAttributes(const QString &name, const QString& value);

  //! Returns the names of all widgets for which this sub-control is valid.
  QStringList possibleSubControlWidgets(const QString& name) const;

  void addSubControl(const QString& control, const QString& widget);
  void addSubControl(const QString& control, QStringList& widgets);
  void removeSubControl(const QString& control);
  void addPseudoState(const QString& state);
  void removePseudoState(const QString& state);

private:
  QStringList m_widgets;
  QStringList m_colors;
  QStringList m_attributeNames;
  QStringList m_properties;
  QStringList m_pseudoStates;
  QStringList m_possibleWidgets;
  QStringList m_StylesheetProperties;
  QStringList m_alignmentValues;
  QStringList m_paletteRoles, m_gradient, m_attachment,m_borderStyle,m_borderImage,
    m_fontStyle, m_fontWeight, m_icon, m_origin, m_outlineStyle;

  QMap<QString, QStringList> m_subControls;
  QMap<QString, AttributeType> m_attributes;
  QMap<QString, AttributeType> m_stylesheetAttributes;

  bool checkPropertyValue(AttributeType propertyAttribute,
                          const QString& valuename,
                          StylesheetData* data = nullptr);
  bool checkAlignment(const QString& value);
  bool checkAttachment(const QString& value);
  bool checkBackground(const QString& value);
  bool checkBool(const QString& value);
  bool checkBoolean(const QString& value);
  bool checkBorder(const QString& value);
  bool checkBorderImage(const QString& value);
  bool checkBorderStyle(const QString& value);
  bool checkBoxColors(const QString& value);
  bool checkBoxLengths(const QString& value);
  bool checkBrush(const QString& value);
  bool checkColor(const QString& value);
  bool checkFontStyle(const QString& value);
  bool checkFont(const QString& value);
  bool checkFontSize(const QString& value);
  bool checkFontWeight(const QString& value);
  bool checkGradient(const QString& value);
  bool checkIcon(const QString& value);
  bool checkLength(const QString& value);
  bool checkNumber(const QString& value);
  bool checkOutline(const QString& value);
  bool checkOrigin(const QString& value);
  bool checkOutlineStyle(const QString& value);
  bool checkOutlineColor(const QString& value);
  bool checkOutlineWidth(const QString& value);
  bool checkOutlineOffset(const QString& value);
  bool checkOutlineRadius(const QString& value);
  bool checkPaletteRole(const QString& value);
  bool checkRadius(const QString& value);
  bool checkRepeat(const QString& value);
  bool checkUrl(const QString& value);
  bool checkPosition(const QString& value);
  bool checkTextDecoration(const QString& value);
  //
  bool checkStylesheetEdit(const QString& value,
                           StylesheetData* data = nullptr);
  bool checkStylesheetEditBad(const QString& value,
                              StylesheetData* data = nullptr);
  bool checkStylesheetFontWeight(const QString& value, StylesheetData* data);

  QMap<QString, QStringList> initialiseSubControlMap();
  QStringList initialiseWidgetList();
  QStringList initialisePropertyList();
  QStringList initialisePseudoStateList();
  QMap<QString, AttributeType> initialiseAttributeMap();
  QStringList initialiseStylesheetProperties();
  QMap<QString, AttributeType> initialiseStylesheetMap();
  QStringList initialiseColorList();

  QStringList addControls(int count, ...);

  QMap<int, QString> fuzzySearch(const QString &name, QStringList list);
//  QMap<int, QString> fuzzyTestColorNames(const QString &value);
  QMap<int, QString> fuzzyTestBrush(const QString &value);
};



#endif // DATASTORE_H
