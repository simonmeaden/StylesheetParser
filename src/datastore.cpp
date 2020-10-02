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
#include "datastore.h"
#include "stylesheetedit_p.h"
#include "stylesheetparser/stylesheetedit.h"

namespace StylesheetEditor {

//! The list of permissible colour strings.
const QString DataStore::RE_COLOUR_NAMES =
  "black|silver|gray|whitesmoke|maroon|red|purple|fuchsia|green|"
  "lime|olivedrab|yellow|navy|blue|teal|aquamarine|orange|aliceblue|"
  "antiquewhite|aqua|azure|beige|bisque|blanchedalmond|blueviolet|"
  "brown|burlywood|cadetblue|chartreuse|chocolate|coral|cornflowerblue|"
  "cornsilk|crimson|darkblue|darkcyan|darkgoldenrod|darkgray|darkgreen|"
  "darkgrey|darkkhaki|darkmagenta|darkolivegreen|darkorange|darkorchid|"
  "darkred|darksalmon|darkseagreen|darkslateblue|darkslategray|darkslategrey|"
  "darkturquoise|darkviolet|deeppink|deepskyblue|dimgray|dimgrey|dodgerblue|"
  "firebrick|floralwhite|forestgreen|gainsboro|ghostwhite|goldenrod|gold|"
  "greenyellow|grey|honeydew|hotpink|indianred|indigo|ivory|khaki|"
  "lavenderblush|lavender|lawngreen|lemonchiffon|lightblue|lightcoral|"
  "lightcyan|lightgoldenrodyellow|lightgray|lightgreen|lightgrey|lightpink|"
  "lightsalmon|lightseagreen|lightskyblue|lightslategray|lightslategrey|"
  "lightsteelblue|lightyellow|limegreen|linen|mediumaquamarine|mediumblue|"
  "mediumorchid|mediumpurple|mediumseagreen|mediumslateblue|mediumspringgreen|"
  "mediumturquoise|mediumvioletred|midnightblue|mintcream|mistyrose|moccasin|"
  "navajowhite|oldlace|olive|orangered|orchid|palegoldenrod|palegreen|"
  "paleturquoise|palevioletred|papayawhip|peachpuff|peru|pink|plum|"
  "powderblue|rosybrown|royalblue|saddlebrown|salmon|sandybrown|seagreen|"
  "seashell|sienna|skyblue|slateblue|slategray|slategrey|snow|springgreen|"
  "steelblue|tan|thistle|tomato|transparent|turquoise|violet|wheat|white|"
  "yellowgreen|rebeccapurple";

DataStore::DataStore(QObject* parent)
  : QObject(parent)
  , m_widgets(initialiseWidgetList())
  , m_properties(initialisePropertyList())
  , m_pseudoStates(initialisePseudoStateList())
  , m_StylesheetProperties(initialiseStylesheetProperties())
  , m_subControls(initialiseSubControlMap())
  , m_attributes(initialiseAttributeMap())
  , m_stylesheetAttributes(initialiseStylesheetMap())
{}

QStringList DataStore::widgets() const
{
  return m_widgets;
}

void DataStore::addWidget(const QString& widget)
{
  if (!m_widgets.contains(widget)) {
    m_widgets.append(widget);
  }
}

void DataStore::removeWidget(const QString& widget)
{
  m_widgets.removeAll(widget);
}

bool DataStore::containsWidget(const QString& name)
{
  // NOT toLower() as widget names are cased.
  return m_widgets.contains(name);
}

bool DataStore::containsProperty(const QString& name)
{
  return m_properties.contains(name.toLower());
}

bool DataStore::containsStylesheetProperty(const QString& name)
{
  return m_StylesheetProperties.contains(name.toLower());
}

bool DataStore::containsPseudoState(const QString& name)
{
  return m_pseudoStates.contains(name.toLower());
}

bool DataStore::containsSubControl(const QString& name)
{
  return m_subControls.contains(name.toLower());
}

bool DataStore::checkAlignment(const QString& value)
{
  if (value == "top" | value == "bottom" || value == "left" ||
      value == "right" || value == "center") {
    return true;
  }

  return false;
}

bool DataStore::checkAttachment(const QString& value)
{
  if (value == "scroll" | value == "fixed") {
    return true;
  }

  return false;
}

bool DataStore::checkBackground(const QString& value)
{
  if (checkBrush(value)) {
    return true;
  }

  if (checkUrl(value)) {
    return true;
  }

  if (checkRepeat(value)) {
    return true;
  }

  if (checkAlignment(value)) {
    return true;
  }

  return false;
}

bool DataStore::checkBool(const QString& value)
{
  if (value == "true" or value == "false") {
    return true;
  }

  return false;
}

bool DataStore::checkBoolean(const QString& value)
{
  if (value == "0" or value == "1") {
    return true;
  }

  return false;
}

bool DataStore::checkBorder(const QString& value)
{
  if (checkBorderStyle(value)) {
    return true;
  }

  if (checkLength(value)) {
    return true;
  }

  if (checkBrush(value)) {
    return true;
  }

  return false;
}

bool DataStore::checkBorderImage(const QString& value)
{
  if (checkUrl(value)) {
    return true;
  }

  if (checkNumber(value)) {
    return true;
  }

  if (value == "stretch" || value == "repeat") {
    return true;
  }

  return false;
}

bool DataStore::checkBorderStyle(const QString& value)
{
  if (value == "dashed" | value == "dot-dash" || value == "dot-dot-dash" ||
      value == "dotted" || value == "double" || value == "groove" ||
      value == "inset" || value == "outset" || value == "ridge" ||
      value == "solid" || value == "none") {
    return true;
  }

  return false;
}

bool DataStore::checkBoxColors(const QString& value)
{
  return checkColor(value);
}

bool DataStore::checkBoxLengths(const QString& value)
{
  return checkLength(value);
}

bool DataStore::checkBrush(const QString& value)
{
  if (checkColor(value)) {
    return true;
  }

  if (checkGradient(value)) {
    return true;
  }

  if (checkPaletteRole(value)) {
    return true;
  }

  return false;
}

bool DataStore::checkColor(const QString& value)
{
  QRegularExpression re;
  QRegularExpressionMatch match;

  // check if this was a color name.
  re.setPattern(RE_COLOUR_NAMES);
  match = re.match(value);

  if (match.hasMatch()) {
    return true;
  }

  if (value.startsWith("#")) {
    if (value.length() != 4 || value.length() != 7) {
      return false;
    }

    bool ok = false;
    QString val = value.right(value.length() - 1);
    value.toUInt(&ok, 16);

    if (ok) {
      return ok;
    }
  }

  if (value.startsWith("rgba")) {
    QString rgb = value.mid(value.indexOf('('), value.indexOf(')'));
    QStringList cols = rgb.split(',');

    if (cols.count() != 4) {
      return false;
    }

    bool ok = false;
    int val, count = 0;

    for (int i = 0; i < 4; i++) {
      auto col = cols.at(i).trimmed();

      if (col.endsWith('%')) {
        col = col.left(col.length() - 1);
        val = col.toInt(&ok);

        if (val >= 0 && val <= 100) {
          count++;
        }

      } else {
        val = col.toInt(&ok);

        if (val >= 0 && val <= 255) {
          count++;
        }
      }
    }

    if (count == 4) {
      return true;
    }
  }

  if (value.startsWith("rgb")) { // include rgba in this case
    QString rgb = value.mid(value.indexOf('('), value.indexOf(')'));
    QStringList cols = rgb.split(',');

    if (cols.count() != 3) {
      return false;
    }

    bool ok = false;
    int val, count = 0;

    for (int i = 0; i < 3; i++) {
      auto col = cols.at(i).trimmed();

      if (col.endsWith('%')) {
        col = col.left(col.length() - 1);
        val = col.toInt(&ok);

        if (val >= 0 && val <= 100) {
          count++;
        }

      } else {
        val = col.toInt(&ok);

        if (val >= 0 && val <= 255) {
          count++;
        }
      }
    }

    if (count == 3) {
      return true;
    }
  }

  if (value.startsWith("hsla") || value.startsWith("hsva")) {
    QString rgb = value.mid(value.indexOf('('), value.indexOf(')'));
    QStringList cols = rgb.split(',');

    if (cols.count() != 4) {
      return false;
    }

    bool ok = false;
    int val, count = 0;

    for (int i = 0; i < 4; i++) {
      auto col = cols.at(i).trimmed();

      if (col.endsWith('%')) {
        col = col.left(col.length() - 1);
        val = col.toInt(&ok);

        if (i == 0) {
          // hue cannot be a percentage.
          // a percentage here will cause a fail.
          continue;

        } else if (val >= 0 && val <= 100) {
          count++;
        }

      } else {
        val = col.toInt(&ok);

        if (i == 0 && val >= 0 && val < 360) {
          count++;

        } else if (val >= 0 && val <= 100) {
          count++;
        }
      }
    }

    if (count == 4) {
      return true;
    }
  }

  if (value.startsWith("hsl") || value.startsWith("hsv")) {
    QString rgb = value.mid(value.indexOf('('), value.indexOf(')'));
    QStringList cols = rgb.split(',');

    if (cols.count() != 3) {
      return false;
    }

    bool ok = false;
    int val, count = 0;

    for (int i = 0; i < 3; i++) {
      auto col = cols.at(i).trimmed();

      if (col.endsWith('%')) {
        col = col.left(col.length() - 1);
        val = col.toInt(&ok);

        if (i == 0) {
          // hue cannot be a percentage.
          // a percentage here will cause a fail.
          continue;

        } else if (val >= 0 && val <= 100) {
          count++;
        }

      } else {
        val = col.toInt(&ok);

        if (i == 0 && val >= 0 && val < 360) {
          count++;

        } else if (val >= 0 && val <= 100) {
          count++;
        }
      }
    }

    if (count == 3) {
      return true;
    }
  }

  return false;
}

bool DataStore::checkFontStyle(const QString& value)
{
  if (value == "normal" | value == "italic" || value == "oblique") {
    return true;
  }

  return false;
}

bool DataStore::checkFont(const QString& value)
{
  if (checkFontSize(value)) {
    return true;
  }

  if (checkFontStyle(value)) {
    return true;
  }

  if (checkFontWeight(value)) {
    return true;
  }

  // TODO check that the string is a valid font string.
  return false;
}

bool DataStore::checkFontSize(const QString& value)
{
  return checkLength(value);
}

bool DataStore::checkFontWeight(const QString& value)
{
  if (value == "normal" || value == "bold" || value == "bolder" ||
      value == "lighter" || value == "100" || value == "200" ||
      value == "300" || value == "400" || value == "500" || value == "600" ||
      value == "700" || value == "800" || value == "900") {
    return true;
  }

  return false;
}

bool DataStore::checkGradient(const QString& value)
{
  if (value == "qlineargradient" | value == "qradialgradient" ||
      value == "qconicalgradient") {
    return true;
  }

  return false;
}

bool DataStore::checkIcon(const QString& value)
{
  if (checkUrl(value)) {
    return true;
  }

  if (value == "disabled" | value == "active" || value == "normal" ||
      value == "selected" || value == "on" || value == "off") {
    return true;
  }

  return false;
}

bool DataStore::checkLength(const QString& value)
{
  bool ok = false;

  if (value.endsWith("px") || value.endsWith("pt") || value.endsWith("em") ||
      value.endsWith("ex")) {
    value.left(value.length() - 2).toUInt(&ok);

  } else if (value.at(value.length() - 1).isDigit()) {
    value.left(value.length() - 2).toUInt(&ok);
  }

  if (ok) {
    return true;
  }

  return false;
}

bool DataStore::checkNumber(const QString& value)
{
  bool ok = false;
  // don't actually need the value, I only want to know
  // if it actually is a number.
  value.toDouble(&ok);
  return ok;
}

bool DataStore::checkOutline(const QString& value)
{
  if (checkOutlineColor(value)) {
    return true;
  }

  if (checkOutlineStyle(value)) {
    return true;
  }

  if (checkOutlineWidth(value)) {
    return true;
  }

  if (checkOutlineOffset(value)) {
    return true;
  }

  return false;
}

bool DataStore::checkOrigin(const QString& value)
{
  if (value == "margin" | value == "border" || value == "padding" ||
      value == "content") {
    return true;
  }

  return false;
}

bool DataStore::checkOutlineStyle(const QString& value)
{
  if (value == "dotted" | value == "solid" || value == "double" ||
      value == "groove" || value == "ridge" || value == "inset" ||
      value == "outset" || value == "none" || value == "hidden") {
    return true;
  }

  return false;
}

bool DataStore::checkOutlineColor(const QString& value)
{
  if (value == "invert") {
    return true;
  }

  return checkColor(value);
}

bool DataStore::checkOutlineWidth(const QString& value)
{
  if (value == "thin" || value == "medium" or value == "thick") {
    return true;
  }

  return checkLength(value);
}

bool DataStore::checkOutlineOffset(const QString& value)
{
  return checkLength(value);
}

bool DataStore::checkOutlineRadius(const QString& value)
{
  // check for 4 radius values.
  return checkRadius(value);
}

bool DataStore::checkPaletteRole(const QString& value)
{
  if (value == "alternate-base" | value == "base" || value == "bright-text" ||
      value == "button" || value == "button-text" || value == "dark" ||
      value == "highlight" || value == "highlighted-text" || value == "light" ||
      value == "link" || value == "link-visited" || value == "mid" ||
      value == "midlight" || value == "shadow" || value == "text" ||
      value == "window" || value == "window-text") {
    return true;
  }

  return false;
}

bool DataStore::checkRadius(const QString& value)
{
  return checkLength(value);
}

bool DataStore::checkRepeat(const QString& value)
{
  if (value == "repeat-x" | value == "repeat-y" || value == "repeat" ||
      value == "no-repeat") {
    return true;
  }

  return false;
}

bool DataStore::checkUrl(const QString& value)
{
  QFile file(value);

  if (file.exists()) {
    return true;
  }

  // TODO add checking for valid Qt resource?
  if (value.startsWith(":/")) {
    return true;
  }

  return false;
}

bool DataStore::checkPosition(const QString& value)
{
  if (value == "relative" || value == "absolute") {
    return true;
  }

  return false;
}

bool DataStore::checkTextDecoration(const QString& value)
{
  if (value == "none" || value == "underline" || value == "overline" ||
      value == "line-through") {
    return true;
  }

  return false;
}

bool DataStore::checkStylesheetEdit(const QString& value, StylesheetData* data)
{
  if (data && checkColor(value)) {
    data->colors.append(value);
    return true;
  }

  if (checkStylesheetFontWeight(value, data)) {
    return true;
  }

  return false;
}

bool DataStore::checkStylesheetEditBad(const QString& value, StylesheetData* data)
{
  if (data) {
    if (checkColor(value)) {
      data->colors.append(QColor(value));
      return true;
    }

    if (checkStylesheetFontWeight(value, data)) {
      return true;
    }

    if (value == "none") {
      data->underline.append(QTextCharFormat::NoUnderline);
      return true;

    } else if (value == "single") {
      data->underline.append(QTextCharFormat::SingleUnderline);
      return true;

    } else if (value == "dash") {
      data->underline.append(QTextCharFormat::DashUnderline);
      return true;

    } else if (value == "dot") {
      data->underline.append(QTextCharFormat::DotLine);
      return true;

    } else if (value == "dashdot") {
      data->underline.append(QTextCharFormat::DashDotLine);
      return true;

    } else if (value == "dashdotdot") {
      data->underline.append(QTextCharFormat::DashDotDotLine);
      return true;

    } else if (value == "wave") {
      data->underline.append(QTextCharFormat::WaveUnderline);
      return true;

    } else if (value == "spellcheck") {
      data->underline.append(QTextCharFormat::SpellCheckUnderline);
      return true;
    }
  }

  return false;
}

bool DataStore::checkStylesheetFontWeight(const QString& value, StylesheetData* data)
{
  if (data) {
    if (value == "thin") {
      data->weights.append(QFont::Thin);
      return true;

    } else if (value == "extralight") {
      data->weights.append(QFont::ExtraLight);
      return true;

    } else if (value == "light") {
      data->weights.append(QFont::Light);
      return true;

    } else if (value == "normal") {
      data->weights.append(QFont::Normal);
      return true;

    } else if (value == "medium") {
      data->weights.append(QFont::Medium);
      return true;

    } else if (value == "demibold") {
      data->weights.append(QFont::DemiBold);
      return true;

    } else if (value == "bold") {
      data->weights.append(QFont::Bold);
      return true;

    } else if (value == "extrabold") {
      data->weights.append(QFont::ExtraBold);
      return true;

    } else if (value == "black") {
      data->weights.append(QFont::Black);
      return true;
    }
  }

  return false;
}

bool DataStore::checkPropertyValue(DataStore::AttributeType propertyAttribute,
                                   const QString& valuename,
                                   StylesheetData* data)
{
  switch (propertyAttribute) {
  case Alignment:
    return checkAlignment(valuename);

  case Attachment:
    return checkAttachment(valuename);

  case Background:
    return checkBackground(valuename);

  case Bool:
    return checkBool(valuename);

  case Boolean:
    return checkBoolean(valuename);

  case Border:
    return checkBorder(valuename);

  case BorderImage:
    return checkBorderImage(valuename);

  case BorderStyle:
    return checkBorderStyle(valuename);

  case BoxColors:
    return checkBoxColors(valuename);

  case BoxLengths:
    return checkBoxLengths(valuename);

  case Brush:
    return checkBrush(valuename);

  case Color:
    return checkColor(valuename);

  case Font:
    return checkFont(valuename);

  case FontSize:
    return checkFontSize(valuename);

  case FontStyle:
    return checkFontStyle(valuename);

  case FontWeight:
    return checkFontWeight(valuename);

  case Gradient:
    return checkGradient(valuename);

  case Icon:
    return checkIcon(valuename);

  case Length:
    return checkLength(valuename);

  case Number: // TODO not supported.
    return checkNumber(valuename);

  case Origin:
    return checkOrigin(valuename);

  case Outline:
    return checkOutline(valuename);

  case OutlineRadius:
    return checkOutlineRadius(valuename);

  case OutlineStyle:
    return checkOutlineStyle(valuename);

  case PaletteRole:
    return checkPaletteRole(valuename);

  case Position:
    return checkPosition(valuename);

  case Radius:
    return checkRadius(valuename);

  case Repeat:
    return checkRepeat(valuename);

  case TextDecoration:
    return checkTextDecoration(valuename);

  case Url:
    return checkUrl(valuename);

  case String:
    // value IS a string.
    // might need to check the string contents here.
    return true;

  case List:
    // This should never reach here.
    break;

  case StylesheetEdit:
    return checkStylesheetEdit(valuename, data);

  case StylesheetEditBad:
    return checkStylesheetEditBad(valuename, data);
  }

  return false;
}

bool DataStore::getIfValidStylesheetValue(const QString& propertyname,
    const QString& valuename,
    StylesheetData* data)
{
  if (valuename.isEmpty()) {
    return false;
  }

  AttributeType stylesheetAttribute =
    m_stylesheetAttributes.value(propertyname);
  return checkPropertyValue(stylesheetAttribute, valuename, data);
}

bool DataStore::isValidPropertyValue(const QString& propertyname,
                                     const QString& valuename)
{
  if (valuename.isEmpty()) {
    return false;
  }

  AttributeType propertyAttribute = m_attributes.value(propertyname);
  return checkPropertyValue(propertyAttribute, valuename, new StylesheetData());
}

QList<bool> DataStore::isValidPropertyValues(const QString& name, const QStringList& values)
{
  QList<bool> results;

  if (values.length() == 1) {
    results.append(isValidPropertyValue(name, values.at(0)));

  } else {
    AttributeType type = m_attributes.value(name);

    for (auto value : values) {
      switch (type) {
      case BoxColors:
        if (values.length() == 0 || values.length() > 4) {
          results.append(false);

        } else {
          results.append(checkColor(value));
        }

        break;

      case BoxLengths:
        if (values.length() == 0 || values.length() > 4) {
          results.append(false);

        } else {
          results.append(checkLength(value));
        }

        break;

      case Radius:
        if (values.length() == 0 || values.length() > 2) {
          results.append(false);

        } else {
          results.append(checkRadius(value));
        }

        break;

      default:
        break;
      }
    }
  }

  return results;
}

QStringList DataStore::possibleSubControlWidgets(const QString& name) const
{
  return m_subControls.value(name);
}

void DataStore::addSubControl(const QString& control, const QString& widget)
{
  m_subControls.insert(control, addControls(1, &widget));
}

void DataStore::addSubControl(const QString& control, QStringList& widgets)
{
  m_subControls.insert(control, widgets);
}

void DataStore::removeSubControl(const QString& control)
{
  if (m_subControls.contains(control)) {
    m_subControls.remove(control);
  }
}

void DataStore::addPseudoState(const QString& state)
{
  if (!m_pseudoStates.contains(state)) {
    m_pseudoStates.append(state);
  }
}

void DataStore::removePseudoState(const QString& state)
{
  m_pseudoStates.removeAll(state);
}

// QMap<QString, AttributeTypes> DataStore::attributes() const
//{
//  return m_attributes;
//}

// void DataStore::setAttributes(const QMap<QString, AttributeTypes>&
// attributes)
//{
//  m_attributes = attributes;
//}

QMap<QString, QStringList> DataStore::initialiseSubControlMap()
{
  QMap<QString, QStringList> map;
  map.insert("add-line", addControls(1, new QString("QScrollBar")));
  map.insert("add-page", addControls(1, new QString("QScrollBar")));
  map.insert("branch", addControls(1, new QString("QTreeBar")));
  map.insert("chunk", addControls(1, new QString("QProgressBar")));
  map.insert(
    "close-button",
    addControls(2, new QString("QDockWidget"), new QString("QTabBar")));
  map.insert("corner", addControls(1, new QString("QAbstractScrollArea")));
  map.insert("down-arrow",
             addControls(4,
                         new QString("QComboBox"),
                         new QString("QHeaderView"),
                         new QString("QScrollBar"),
                         new QString("QSpinBox")));
  map.insert(
    "down-button",
    addControls(2, new QString("QScrollBar"), new QString("QSpinBox")));
  map.insert("drop-down", addControls(1, new QString("QComboBox")));
  map.insert("float-button", addControls(1, new QString("QDockWidget")));
  map.insert("groove", addControls(1, new QString("QSlider")));
  map.insert("indicator",
             addControls(5,
                         new QString("QAbstractItemView"),
                         new QString("QCheckBox"),
                         new QString("QRadioButton"),
                         new QString("QMenu"),
                         new QString("QGroupBox")));
  map.insert("handle",
             addControls(3,
                         new QString("QScrollBar"),
                         new QString("QSplitter"),
                         new QString("QSlider")));
  map.insert(
    "icon",
    addControls(2, new QString("QAbstractItemView"), new QString("QMenu")));
  map.insert("item",
             addControls(4,
                         new QString("QAbstractItemView"),
                         new QString("QMenuBar"),
                         new QString("QMenu"),
                         new QString("QStatusBar")));
  map.insert("left-arrow", addControls(1, new QString("QScrollBar")));
  map.insert("left-corner", addControls(1, new QString("QTabWidget")));
  map.insert("menu-arrow", addControls(1, new QString("QToolButton")));
  map.insert("menu-button", addControls(1, new QString("QToolButton")));
  map.insert("menu-indicator", addControls(1, new QString("QPushButton")));
  map.insert("right-arrow",
             addControls(2, new QString("QMenu"), new QString("QScrollBar")));
  map.insert("pane", addControls(1, new QString("QTabWidget")));
  map.insert("right-corner", addControls(1, new QString("QTabWidget")));
  map.insert("scroller",
             addControls(2, new QString("QMenu"), new QString("QTabBar")));
  map.insert("section", addControls(1, new QString("QHeaderView")));
  map.insert("separator",
             addControls(2, new QString("QMenu"), new QString("QMainWindow")));
  map.insert("sub-line", addControls(1, new QString("QScrollBar")));
  map.insert("sub-page", addControls(1, new QString("QScrollBar")));
  map.insert("tab",
             addControls(2, new QString("QTabBar"), new QString("QToolBox")));
  map.insert("tab-bar", addControls(1, new QString("QTabWidget")));
  map.insert("tear", addControls(1, new QString("QTabBar")));
  map.insert("tearoff", addControls(1, new QString("QMenu")));
  map.insert("text", addControls(1, new QString("QAbstractItemView")));
  map.insert(
    "title",
    addControls(2, new QString("QGroupBox"), new QString("QDockWidget")));
  map.insert("up-arrow",
             addControls(3,
                         new QString("QHeaderView"),
                         new QString("QScrollBar"),
                         new QString("QSpinBox")));
  map.insert("up-button", addControls(1, new QString("QSpinBox")));
  return map;
}

QStringList DataStore::initialiseWidgetList()
{
  QStringList list;
  list << "QAbstractScrollArea"
       << "QCheckBox"
       << "QColumnView"
       << "QComboBox"
       << "QDateEdit"
       << "QDateTimeEdit"
       << "QDialog"
       << "QDialogButtonBox"
       << "QDockWidget"
       << "QDoubleSpinBox"
       << "QFrame"
       << "QGroupBox"
       << "QHeaderView"
       << "QLabel"
       << "QLineEdit"
       << "QListView"
       << "QListWidget"
       << "QMainWindow"
       << "QMenu"
       << "QMenu"
       << "QMessageBox"
       << "QProgressBar"
       << "QPushButton"
       << "QRadioButton"
       << "QScrollBar"
       << "QSizeGrip"
       << "QSlider"
       << "QSpinBox"
       << "QSplitter"
       << "QStatusBar"
       << "QTabBar"
       << "QTabWidget"
       << "QTableView"
       << "QTableWidget"
       << "QTextEdit"
       << "QTimeEdit"
       << "QToolBar"
       << "QToolButton"
       << "QToolBox"
       << "QToolTip"
       << "QTreeView"
       << "QTreeWidget"
       << "QWidget"
       // I might as well add stylesheetedit stuff to this widget.
       << "StylesheetEdit";
  return list;
}

QStringList DataStore::initialisePseudoStateList()
{
  QStringList list;
  list << "active"
       << "adjoins-item"
       << "alternate"
       << "bottom"
       << "checked"
       << "closable"
       << "closed"
       << "default"
       << "disabled"
       << "editable"
       << "edit-focus"
       << "enabled"
       << "exclusive"
       << "first"
       << "flat"
       << "floatable"
       << "focus"
       << "has-children"
       << "has-siblings"
       << "horizontal"
       << "hover"
       << "indeterminate"
       << "last"
       << "left"
       << "maximized"
       << "middle"
       << "minimized"
       << "movable"
       << "no-frame"
       << "non-exclusive"
       << "off"
       << "on"
       << "only-one"
       << "open"
       << "next-selected"
       << "pressed"
       << "previous-selected"
       << "read-only"
       << "right"
       << "selected"
       << "top"
       << "unchecked"
       << "vertical"
       << "window";
  return list;
}

QStringList DataStore::initialisePropertyList()
{
  QStringList list;
  list << "alternate-background-color"
       << "background"
       << "background-color"
       << "background-image"
       << "background-repeat"
       << "background-position"
       << "background-attachment"
       << "background-clip"
       << "background-origin"
       << "border"
       << "border-top"
       << "border-right"
       << "border-bottom"
       << "border-left"
       << "border-color"
       << "border-top-color"
       << "border-right-color"
       << "border-bottom-color"
       << "border-left-color"
       << "border-image"
       << "border-radius"
       << "border-top-left-radius"
       << "border-top-right-radius"
       << "border-bottom-right-radius"
       << "border-bottom-left-radius"
       << "border-style"
       << "border-top-style"
       << "border-right-style"
       << "border-bottom-style"
       << "border-left-style"
       << "border-width"
       << "border-top-width"
       << "border-right-width"
       << "border-bottom-width"
       << "border-left-width"
       << "bottom"
       << "button-layout"
       << "color"
       << "dialogbuttonbox-buttons-have-icons"
       << "font"
       << "font-family"
       << "font-size"
       << "font-style"
       << "font-weight"
       << "gridline-color"
       << "height"
       << "icon"
       << "icon-size"
       << "image"
       << "image-position"
       << "left"
       << "lineedit-password-character"
       << "lineedit-password-mask-delay"
       << "margin"
       << "margin-top"
       << "margin-right"
       << "margin-bottom"
       << "margin-left"
       << "max-height"
       << "max-width"
       << "messagebox-text-interaction-flags"
       << "min-height"
       << "min-width"
       << "opacity"
       << "outline"
       << "outline-color"
       << "outline-offset"
       << "outline-style"
       << "outline-radius"
       << "outline-bottom-left-radius"
       << "outline-bottom-right-radius"
       << "outline-top-left-radius"
       << "outline-top-right-radius"
       << "padding"
       << "padding-top"
       << "padding-right"
       << "padding-bottom"
       << "padding-left"
       << "paint-alternating-row-colors-for-empty-area"
       << "position"
       << "right"
       << "selection-background-color"
       << "selection-color"
       << "show-decoration-selected"
       << "spacing"
       << "subcontrol-origin"
       << "subcontrol-position"
       << "titlebar-show-tooltips-on-buttons"
       << "widget-animation-duration"
       << "text-align"
       << "text-decoration"
       << "top"
       << "width"
       << "-qt-background-role"
       << "-qt-style-features"
       // I might as well add stylesheet stuff for this widget.
       << "widget"
       << "subcontrol"
       << "subcontrolmarker"
       << "pseudostate"
       << "pseudostatemarker"
       << "property"
       << "propertymarker"
       << "value"
       << "startbrace"
       << "endbrace"
       << "bracematch"
       << "comment"
       << "bad";
  return list;
}

QStringList DataStore::initialiseStylesheetProperties()
{
  QStringList list;
  list << "widget"
       << "subcontrol"
       << "subcontrolmarker"
       << "pseudostate"
       << "pseudostatemarker"
       << "property"
       << "propertymarker"
       << "value"
       << "startbrace"
       << "endbrace"
       << "bracematch"
       << "comment"
       << "bad";
  // TODO more values
  return list;
}

QMap<QString, DataStore::AttributeType> DataStore::initialiseStylesheetMap()
{
  QMap<QString, AttributeType> map;
  map.insert("widget", StylesheetEdit);
  map.insert("subcontrol", StylesheetEdit);
  map.insert("pseudostate", StylesheetEdit);
  map.insert("subcontrolmarker", StylesheetEdit);
  map.insert("pseudostatemarker", StylesheetEdit);
  map.insert("property", StylesheetEdit);
  map.insert("propertymarker", StylesheetEdit);
  map.insert("value", StylesheetEdit);
  map.insert("startbrace", StylesheetEdit);
  map.insert("endbrace", StylesheetEdit);
  map.insert("bracematch", StylesheetEdit);
  map.insert("comment", StylesheetEdit);
  map.insert("bad", StylesheetEditBad);
  // TODO more values
  return map;
}

QMap<QString, DataStore::AttributeType> DataStore::initialiseAttributeMap()
{
  QMap<QString, AttributeType> map;
  map.insert("alternate-background-color", Brush);
  map.insert("background", Background);
  map.insert("background-color", Brush);
  map.insert("background-image", Url);
  map.insert("background-repeat", Repeat);
  map.insert("background-position", Alignment);
  map.insert("background-clip", Origin);
  map.insert("background-origin", Origin);
  map.insert("border", Border);
  map.insert("border-top", Border);
  map.insert("border-left", Border);
  map.insert("border-right", Border);
  map.insert("border-bottom", Border);
  map.insert("border-color", BoxColors);
  map.insert("border-top-color", Brush);
  map.insert("border-right-color", Brush);
  map.insert("border-bottom-color", Brush);
  map.insert("border-left-color", Brush);
  map.insert("border-image", BorderImage);
  map.insert("border-radius", Radius);
  map.insert("border-top-left-radius", Radius);
  map.insert("border-top-right-radius", Radius);
  map.insert("border-bottom-right-radius", Radius);
  map.insert("border-bottom-left-radius", Radius);
  map.insert("border-style", BorderStyle);
  map.insert("border-top-style", BorderStyle);
  map.insert("border-right-style", BorderStyle);
  map.insert("border-bottom-style", BorderStyle);
  map.insert("border-left-style", BorderStyle);
  map.insert("border-width", BoxLengths);
  map.insert("border-top-width", Length);
  map.insert("border-right-width", Length);
  map.insert("border-bottom-width", Length);
  map.insert("border-left-width", Length);
  map.insert("bottom", Length);
  map.insert("button-layout", Number);
  map.insert("color", Brush);
  map.insert("dialogbuttonbox-buttons-have-icons", Brush);
  map.insert("font", Font);
  map.insert("font-family", String);
  map.insert("font-size", FontSize);
  map.insert("font-style", FontStyle);
  map.insert("font-weight", FontWeight);
  map.insert("gridline-color", Color);
  map.insert("height", Length);
  map.insert("icon", Url);
  map.insert("icon-size", Length);
  map.insert("image", Alignment);
  map.insert("image-position", Url);
  map.insert("left", Length);
  map.insert("lineedit-password-character", Number);
  map.insert("lineedit-password-mask-delay", Number);
  map.insert("margin", BoxLengths);
  map.insert("margin-top", Length);
  map.insert("margin-right", Length);
  map.insert("margin-bottom", Length);
  map.insert("margin-left", Length);
  map.insert("max-height", Length);
  map.insert("max-width", Length);
  map.insert("messagebox-text-interaction-flags", Number);
  map.insert("min-height", Length);
  map.insert("min-width", Length);
  map.insert("opacity", Length);
  map.insert("outline", Outline); // not known??
  map.insert("outline-color", Color);
  map.insert("outline-offset", Length);
  map.insert("outline-style", OutlineStyle);
  map.insert("outline-radius", OutlineRadius);
  map.insert("outline-bottom-left-radius", Radius);
  map.insert("outline-bottom-right-radius", Radius);
  map.insert("outline-top-left-radius", Radius);
  map.insert("outline-top-right-radius", Radius);
  map.insert("padding", BoxLengths);
  map.insert("padding-top", Length);
  map.insert("padding-right", Length);
  map.insert("padding-bottom", Length);
  map.insert("padding-left", Length);
  map.insert("paint-alternating-row-colors-for-empty-area", Bool);
  map.insert("position", Position);
  map.insert("right", Length);
  map.insert("selection-background-color", Brush);
  map.insert("selection-color", Brush);
  map.insert("show-decoration-selected", Boolean);
  map.insert("spacing", Length);
  map.insert("subcontrol-position", Alignment);
  map.insert("titlebar-show-tooltips-on-buttons", Bool);
  map.insert("widget-animation-duration", Number);
  map.insert("text-align", Alignment);
  map.insert("text-decoration", TextDecoration);
  map.insert("top", Length);
  map.insert("width", Length);
  map.insert("-qt-background-role", PaletteRole);
  map.insert("-qt-style-features", List);
  // Below this is the attributes available for this stylesheet editor.
  map.insert("widget", StylesheetEdit);
  map.insert("subcontrol", StylesheetEdit);
  map.insert("pseudostate", StylesheetEdit);
  map.insert("subcontrolmarker", StylesheetEdit);
  map.insert("pseudostatemarker", StylesheetEdit);
  map.insert("property", StylesheetEdit);
  map.insert("propertymarker", StylesheetEdit);
  map.insert("value", StylesheetEdit);
  map.insert("startbrace", StylesheetEdit);
  map.insert("endbrace", StylesheetEdit);
  map.insert("bracematch", StylesheetEdit);
  map.insert("comment", StylesheetEdit);
  map.insert("bad", StylesheetEditBad);

  return map;
}

QStringList DataStore::addControls(int count, ...)
{
  va_list arguments;

  QStringList widgets;
  va_start(arguments, count);

  for (int i = 0; i < count; i++) {
    QString* currentString = va_arg(arguments, QString*);
    widgets << *currentString;
  }

  va_end(arguments);

  return widgets;
}

} // end of StylesheetParser
