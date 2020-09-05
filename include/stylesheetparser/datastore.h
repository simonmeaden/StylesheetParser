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
#include <QRegularExpression>
#include <QFile>

namespace StylesheetParser {

class DataStore : public QObject
{
  Q_OBJECT
public:
  enum AttributeType
  {
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
  };

  explicit DataStore(QObject* parent);

  QStringList widgets() const;

  void addWidget(const QString& widget);
  void removeWidget(const QString& widget);
  bool containsWidget(const QString& name);
  bool containsProperty(const QString& name);
  bool containsPseudoState(const QString& name);
  bool containsSubControl(const QString& name);

  bool isValidPropertyValue(const QString& name, const QString& value);
  QList<bool> isValidPropertyValues(const QString& name, const QStringList& values);

  //! Returns the names of all widgets for which this sub-control is valid.
  QStringList possibleSubControlWidgets(const QString& name) const;

  void addSubControl(const QString& control, const QString& widget);
  void addSubControl(const QString& control, QStringList& widgets);
  void removeSubControl(const QString& control);
  void addPseudoState(const QString& state);
  void removePseudoState(const QString& state);

  //  QMap<QString, AttributeTypes> attributes() const;
  //  void setAttributes(const QMap<QString, AttributeTypes>& attributes);

private:
  QStringList m_widgets;
  QStringList m_properties;
  QStringList m_pseudoStates;
  QStringList m_possibleWidgets;
  QMap<QString, QStringList> m_subControls;
  QMap<QString, AttributeType> m_attributes;
  QMap<QString, QStringList> m_attributeValues;

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

  QMap<QString, QStringList> initialiseSubControlMap();
  QStringList initialiseWidgetList();
  QStringList initialisePropertyList();
  QStringList initialisePseudoStateList();
  QMap<QString, AttributeType> initialiseAttributeMap();

  QStringList addControls(int count, ...);

  static const QString RE_COLOUR_NAMES;
};


} // end of StylesheetParser

#endif // DATASTORE_H
