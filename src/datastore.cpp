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
#include "node.h"
#include "parser.h"
#include "stylesheetedit_p.h"
#include "stylesheetparser/stylesheetedit.h"

DataStore::DataStore(QObject* parent)
  : QObject(parent)
  , m_invalidIcon(":/icons/invalid")
  , m_validIcon(":/icons/valid")
  , m_addSemiColonIcon(":/icons/add-scolon")
  , m_addDColonIcon(":/icons/add-dcolon")
  , m_addColonIcon(":/icons/add-colon")
  , m_badSColonIcon(":/icons/bad-scolon")
  , m_badColonIcon(":/icons/bad-colon")
  , m_badDColonIcon(":/icons/bad-dcolon")
  , m_noIcon(":/icons/no")
  , m_fuzzyIcon(":/icons/fuzzy")
  , m_widgets(initialiseWidgetList())
  , m_colors(initialiseColorList())
  , m_properties(initialisePropertyList())
  , m_pseudoStates(initialisePseudoStateList())
  , m_StylesheetProperties(initialiseStylesheetProperties())
  , m_subControls(initialiseSubControlMap())
  , m_attributes(initialiseAttributeMap())
  , m_stylesheetAttributes(initialiseStylesheetMap())
  , m_braceCount(0)
  , m_manualMove(false)
  , m_hasSuggestion(false)
  , m_maxSuggestionCount(30)
{
  m_alignmentValues << "top"
                    << "bottom"
                    << "left"
                    << "right"
                    << "center";
  m_paletteRoles << "alternate-base"
                 << "base"
                 << "bright-text"
                 << "button"
                 << "button-text"
                 << "dark"
                 << "highlight"
                 << "highlighted-text"
                 << "light"
                 << "link"
                 << "link-visited"
                 << "mid"
                 << "midlight"
                 << "shadow"
                 << "text"
                 << "window"
                 << "window-text";
  m_gradient << "qlineargradient"
             << "qradialgradient"
             << "qconicalgradient";
  m_attachment << "scroll"
               << "fixed";
  m_borderStyle << "dashed"
                << "dot-dash"
                << "dot-dot-dash"
                << "dotted"
                << "double"
                << "groove"
                << "inset"
                << "outset"
                << "ridge"
                << "solid"
                << "none";
  m_borderImage << "stretch"
                << "repeat";
  m_fontStyle << "normal"
              << "italic"
              << "oblique";
  m_fontWeight << "normal"
               << "bold"
               << "bolder"
               << "lighter"
               << "100"
               << "200"
               << "300"
               << "400"
               << "500"
               << "600"
               << "700"
               << "800"
               << "900";
  m_icon << "disabled"
         << "active"
         << "normal"
         << "selected"
         << "on"
         << "off";
  m_origin << "dotted"
           << "solid"
           << "double"
           << "groove"
           << "ridge"
           << "inset"
           << "outset"
           << "none"
           << "hidden";
  m_outlineStyle << "dotted"
                 << "solid"
                 << "double"
                 << "groove"
                 << "ridge"
                 << "inset"
                 << "outset"
                 << "none"
                 << "hidden";
  m_outlineColor = "invert";
  m_outlineWidth << "thin"
                 << "medium"
                 << "thick";
  m_position << "relative"
             << "absolute";
  m_repeat << "repeat-x"
           << "repeat-y"
           << "repeat"
           << "no-repeat";
  m_textDecoration << "none"
                   << "underline"
                   << "overline"
                   << "line-through";
}

DataStore::~DataStore()
{
  emit finished();
}

void
DataStore::addWidget(const QString& widget)
{
  QMutexLocker locker(&m_mutex);
  if (!m_widgets.contains(widget)) {
    m_widgets.append(widget);
  }
}

void
DataStore::removeWidget(const QString& widget)
{
  QMutexLocker locker(&m_mutex);
  m_widgets.removeAll(widget);
}

bool
DataStore::containsWidget(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  // NOT toLower() as widget names are cased.
  return m_widgets.contains(name);
}

QMultiMap<int, QString>
DataStore::fuzzySearch(const QString& name, QStringList list)
{
  QMultiMap<int, QString> matches;
  char* pattern = new char[name.size() + 1];
  strcpy(pattern, name.toStdString().c_str());

  int score = 0;

  for (auto valueStr : list) {
    char* value = new char[valueStr.size() + 1];
    strcpy(value, valueStr.toStdString().c_str());

    if (fts::fuzzy_match(pattern, value, score)) {
      matches.insert(score, valueStr);
    }
  }

  return matches;
}

QMultiMap<int, QString>
DataStore::fuzzySearchWidgets(const QString& name)
{
  return fuzzySearch(name, m_widgets);
}

bool
DataStore::containsProperty(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_properties.contains(name.toLower());
}

QMultiMap<int, QString>
DataStore::fuzzySearchProperty(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return fuzzySearch(name, m_properties);
}

QMultiMap<int, QString>
DataStore::fuzzySearchPropertyValue(const QString& name, const QString& value)
{
  QMutexLocker locker(&m_mutex);
  QMap<int, QString> data, dataIn;
  QStringList list;
  auto attribute = m_attributes.value(name);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum,"
  switch (attribute) {
    case Alignment:
      return fuzzySearch(value, m_alignmentValues);

    case Attachment:
      return fuzzySearch(value, m_attachment);

    case Background: {
      data.insert(fuzzyTestBrush(value));

      return data;
    }

    case Border:
      list << m_borderStyle << m_borderImage << m_colors << m_paletteRoles
           << m_gradient;
      return fuzzySearch(value, list);

    case BorderImage:
      return fuzzySearch(value, m_borderImage);

    case BorderStyle:
      return fuzzySearch(value, m_borderStyle);

    case BoxColors:
    case Color:
      return fuzzySearch(value, m_colors);

    case Brush:
      list << m_colors << m_paletteRoles << m_gradient;
      return fuzzySearch(value, list);

    case Font:
      list << m_fontStyle << m_fontWeight;
      return fuzzySearch(value, list);

    case FontStyle:
      return fuzzySearch(value, m_fontStyle);

    case FontWeight:
      return fuzzySearch(value, m_fontWeight);

    case Gradient:
      return fuzzySearch(value, m_gradient);

    case Icon:
      return fuzzySearch(value, m_icon);

    case Origin:
      return fuzzySearch(value, m_origin);

    case Outline:
      list << m_outlineStyle << m_colors << m_outlineColor << m_outlineWidth;
      return fuzzySearch(value, list);

    case OutlineStyle:
      return fuzzySearch(value, m_outlineStyle);

    case PaletteRole:
      return fuzzySearch(value, m_paletteRoles);

    case Position:
      return fuzzySearch(value, m_position);

    case Repeat:
      return fuzzySearch(value, m_repeat);

    case TextDecoration:
      return fuzzySearch(value, m_textDecoration);

    case StylesheetEditGood:
      list << m_colors << "thin"
           << "extralight"
           << "light"
           << "normal"
           << "medium"
           << "demibold"
           << "bold"
           << "extrabold"
           << "black";
      return fuzzySearch(value, list);

    case StylesheetEditBad:
      list << m_colors << "thin"
           << "extralight"
           << "light"
           << "normal"
           << "medium"
           << "demibold"
           << "bold"
           << "extrabold"
           << "black"
           << "none"
           << "single"
           << "dash"
           << "dot"
           << "dashdot"
           << "dashdotdot"
           << "wave"
           << "spellcheck";
      return fuzzySearch(value, list);
  }
#pragma clang diagnostic pop
  return QMap<int, QString>();
}

bool
DataStore::containsStylesheetProperty(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_StylesheetProperties.contains(name.toLower());
}

bool
DataStore::containsPseudoState(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_pseudoStates.contains(name.toLower());
}

QMultiMap<int, QString>
DataStore::fuzzySearchPseudoStates(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return fuzzySearch(name, m_pseudoStates);
}

bool
DataStore::containsSubControl(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_subControls.contains(name.toLower());
}

QMultiMap<int, QString>
DataStore::fuzzySearchSubControl(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return fuzzySearch(name, m_subControls.keys());
}

bool
DataStore::checkAlignment(const QString& value) const
{
  return m_alignmentValues.contains(value);
}

bool
DataStore::checkAttachment(const QString& value) const
{
  return m_attachment.contains(value);
}

bool
DataStore::checkBackground(const QString& value) const
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

bool
DataStore::checkBool(const QString& value) const
{
  if (value == "true" || value == "false") {
    return true;
  }

  return false;
}

bool
DataStore::checkBoolean(const QString& value) const
{
  if (value == "0" || value == "1") {
    return true;
  }

  return false;
}

bool
DataStore::checkBorder(const QString& value) const
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

bool
DataStore::checkBorderImage(const QString& value) const
{
  if (checkUrl(value)) {
    return true;
  }

  if (checkNumber(value)) {
    return true;
  }

  return m_borderImage.contains(value);
}

bool
DataStore::checkBorderStyle(const QString& value) const
{
  return m_borderStyle.contains(value);
}

bool
DataStore::checkBoxColors(const QString& value) const
{
  return checkColor(value);
}

bool
DataStore::checkBoxLengths(const QString& value) const
{
  return checkLength(value);
}

bool
DataStore::checkBrush(const QString& value) const
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

bool
DataStore::checkColor(const QString& value) const
{
  // check if this was a color name.
  if (m_colors.contains(value)) {
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

bool
DataStore::checkFontStyle(const QString& value) const
{
  return m_fontStyle.contains(value);
}

bool
DataStore::checkFont(const QString& value) const
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

bool
DataStore::checkFontSize(const QString& value) const
{
  return checkLength(value);
}

bool
DataStore::checkFontWeight(const QString& value) const
{
  return m_fontWeight.contains(value);
}

bool
DataStore::checkGradient(const QString& value) const
{
  return m_gradient.contains(value);
}

bool
DataStore::checkIcon(const QString& value) const
{
  if (checkUrl(value)) {
    return true;
  }

  return m_icon.contains(value);
}

bool
DataStore::checkLength(const QString& value) const
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

bool
DataStore::checkNumber(const QString& value) const
{
  bool ok = false;
  // don't actually need the value, I only want to know
  // if it actually is a number.
  value.toDouble(&ok);
  return ok;
}

bool
DataStore::checkOutline(const QString& value) const
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

bool
DataStore::checkOrigin(const QString& value) const
{
  return m_origin.contains(value);
}

bool
DataStore::checkOutlineStyle(const QString& value) const
{
  return m_outlineStyle.contains(value);
}

bool
DataStore::checkOutlineColor(const QString& value) const
{
  if (value == m_outlineColor) {
    return true;
  }

  return checkColor(value);
}

bool
DataStore::checkOutlineWidth(const QString& value) const
{
  if (m_outlineWidth.contains(value)) {
    return true;
  }

  return checkLength(value);
}

bool
DataStore::checkOutlineOffset(const QString& value) const
{
  return checkLength(value);
}

bool
DataStore::checkOutlineRadius(const QString& value) const
{
  // check for 4 radius values.
  return checkRadius(value);
}

bool
DataStore::checkPaletteRole(const QString& value) const
{
  if (m_paletteRoles.contains(value)) {
    return true;
  }

  return false;
}

bool
DataStore::checkRadius(const QString& value) const
{
  return checkLength(value);
}

bool
DataStore::checkRepeat(const QString& value) const
{
  return m_repeat.contains(value);
}

bool
DataStore::checkUrl(const QString& value) const
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

bool
DataStore::checkPosition(const QString& value) const
{
  return m_position.contains(value);
}

bool
DataStore::checkTextDecoration(const QString& value) const
{
  return m_textDecoration.contains(value);
}

bool
DataStore::checkStylesheetEdit(const QString& value, StylesheetData* data) const
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

bool
DataStore::checkStylesheetEditBad(const QString& value,
                                  StylesheetData* data) const
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

bool
DataStore::checkStylesheetFontWeight(const QString& value,
                                     StylesheetData* data) const
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

bool
DataStore::checkPropertyValue(AttributeType propertyAttribute,
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

    case StylesheetEditGood:
      return checkStylesheetEdit(valuename, data);

    case StylesheetEditBad:
      return checkStylesheetEditBad(valuename, data);
  }

  return false;
}

bool
DataStore::ifValidStylesheetValue(const QString& propertyname,
                                  const QString& valuename,
                                  StylesheetData* data)
{
  QMutexLocker locker(&m_mutex);
  if (valuename.isEmpty()) {
    return false;
  }

  AttributeType stylesheetAttribute =
    m_stylesheetAttributes.value(propertyname);
  return (checkPropertyValue(stylesheetAttribute, valuename, data) !=
          NoAttributeValue);
}

bool
DataStore::isValidPropertyValueForProperty(const QString& propertyname,
                                           const QString& valuename)
{
  QMutexLocker locker(&m_mutex);
  if (valuename.isEmpty()) {
    return false;
  }

  AttributeType propertyAttribute = m_attributes.value(propertyname);
  return (checkPropertyValue(propertyAttribute,
                             valuename,
                             new StylesheetData()) != NoAttributeValue);
}

QStringList
DataStore::possibleSubControlWidgets(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_subControls.value(name);
}

void
DataStore::addSubControl(const QString& control, const QString& widget)
{
  QMutexLocker locker(&m_mutex);
  m_subControls.insert(control, addControls(1, &widget));
}

void
DataStore::addSubControl(const QString& control, QStringList& widgets)
{
  QMutexLocker locker(&m_mutex);
  m_subControls.insert(control, widgets);
}

void
DataStore::removeSubControl(const QString& control)
{
  QMutexLocker locker(&m_mutex);
  if (m_subControls.contains(control)) {
    m_subControls.remove(control);
  }
}

void
DataStore::addPseudoState(const QString& state)
{
  QMutexLocker locker(&m_mutex);
  if (!m_pseudoStates.contains(state)) {
    m_pseudoStates.append(state);
  }
}

void
DataStore::removePseudoState(const QString& state)
{
  QMutexLocker locker(&m_mutex);
  m_pseudoStates.removeAll(state);
}

int
DataStore::braceCount()
{
  QMutexLocker locker(&m_mutex);
  return m_braceCount;
}

void
DataStore::setBraceCount(int value)
{
  QMutexLocker locker(&m_mutex);
  m_braceCount = value;
}

void
DataStore::incrementBraceCount()
{
  QMutexLocker locker(&m_mutex);
  m_braceCount++;
}

bool
DataStore::decrementBraceCount()
{
  QMutexLocker locker(&m_mutex);
  if (m_braceCount > 0) {
    m_braceCount--;
    return true;
  }
  return false;
}

//bool
//DataStore::isBraceCountZero()
//{
//  return m_braceStack.isEmpty();
//}

//void
//DataStore::pushStartBrace(StartBraceNode* startbrace)
//{
//  m_startbraces.append(startbrace);
//  m_braceStack.push(startbrace);
//}

//void
//DataStore::pushEndBrace(EndBraceNode* endbrace)
//{
//  m_endbraces.append(endbrace);
//  if (!isBraceCountZero()) {
//    auto startbrace = m_braceStack.pop();
//    endbrace->setStartNode(startbrace);
//    startbrace->setEndBrace(endbrace);
//  }
//}

QMap<QTextCursor, Node*>
DataStore::nodes()
{
  QMutexLocker locker(&m_mutex);
  return m_nodes;
}

void
DataStore::insertNode(QTextCursor cursor, Node* node)
{
  QMutexLocker locker(&m_mutex);
  m_nodes.insert(cursor, node);
//  switch (node->type()) {
//    case StartBraceType:
//      pushStartBrace(qobject_cast<StartBraceNode*>(node));
//      break;
//    case EndBraceType:
//      pushEndBrace(qobject_cast<EndBraceNode*>(node));
//  }
}

bool
DataStore::isNodesEmpty()
{
  QMutexLocker locker(&m_mutex);
  return m_nodes.isEmpty();
}

void
DataStore::clearNodes()
{
  QMutexLocker locker(&m_mutex);
  m_nodes.clear();
}

void
DataStore::setNodes(QMap<QTextCursor, Node*> nodes)
{
  QMutexLocker locker(&m_mutex);
  m_nodes = nodes;
}

int
DataStore::maxSuggestionCount()
{
  QMutexLocker locker(&m_mutex);
  return m_maxSuggestionCount;
}

void
DataStore::setMaxSuggestionCount(int maxSuggestionCount)
{
  QMutexLocker locker(&m_mutex);
  m_maxSuggestionCount = maxSuggestionCount;
}

bool
DataStore::hasSuggestion()
{
  QMutexLocker locker(&m_mutex);
  return m_hasSuggestion;
}

void
DataStore::setHasSuggestion(bool suggestion)
{
  QMutexLocker locker(&m_mutex);
  m_hasSuggestion = suggestion;
}

bool
DataStore::isManualMove()
{
  QMutexLocker locker(&m_mutex);
  return m_manualMove;
}

void
DataStore::setManualMove(bool manualMove)
{
  QMutexLocker locker(&m_mutex);
  m_manualMove = manualMove;
}

Node*
DataStore::currentNode()
{
  QMutexLocker locker(&m_mutex);
  return m_currentNode;
}

void
DataStore::setCurrentWidget(WidgetNode* widget)
{
  QMutexLocker locker(&m_mutex);
  m_currentNode = widget;
}

bool
DataStore::isCurrentWidget(WidgetNode* node)
{
  QMutexLocker locker(&m_mutex);
  return (m_currentNode == node);
}

QTextCursor
DataStore::currentCursor()
{
  QMutexLocker locker(&m_mutex);
  return m_currentCursor;
}

void
DataStore::setCurrentCursor(const QTextCursor& currentCursor)
{
  QMutexLocker locker(&m_mutex);
  m_currentCursor = currentCursor;
}

QIcon
DataStore::invalidIcon() const
{
  return m_invalidIcon;
}

QIcon
DataStore::validIcon() const
{
    return m_validIcon;
}

QIcon DataStore::addDColonIcon() const
{
    return m_addDColonIcon;
}

QIcon
DataStore::addSemiColonIcon() const
{
  return m_addSemiColonIcon;
}

QIcon
DataStore::addColonIcon() const
{
  return m_addColonIcon;
}

QIcon
DataStore::badSemiColonIcon() const
{
    return m_badSColonIcon;
}

QIcon DataStore::badColonIcon() const
{
    return m_badColonIcon;
}

QIcon DataStore::badDColonIcon() const
{
    return m_badDColonIcon;
}

QIcon
DataStore::noIcon() const
{
    return m_noIcon;
}

QIcon DataStore::fuzzyIcon() const
{
    return m_fuzzyIcon;
}

AttributeType
DataStore::propertyValueAttribute(const QString& value)
{
  QMutexLocker locker(&m_mutex);
  if (checkColor(value)) {
    return Color;

  } else if (checkLength(value)) {
    return Length;

  } else if (checkBorder(value)) {
    return Border;

  } else if (checkFont(value)) {
    return Font;

  } else if (checkFontWeight(value)) {
    return FontWeight;

  } else if (checkRadius(value)) {
    return Radius;

  } else if (checkBrush(value)) {
    return Brush;

  } else if (checkFontStyle(value)) {
    return FontStyle;

  } else if (checkFontSize(value)) {
    return FontSize;

  } else if (checkAlignment(value)) {
    return Alignment;

  } else if (checkAttachment(value)) {
    return Attachment;

  } else if (checkBackground(value)) {
    return Background;

  } else if (checkBool(value)) {
    return Bool;

  } else if (checkBoolean(value)) {
    return Boolean;

  } else if (checkBorderImage(value)) {
    return BorderImage;

  } else if (checkBorderStyle(value)) {
    return BorderStyle;

  } else if (checkBoxColors(value)) {
    return BoxColors;

  } else if (checkBoxLengths(value)) {
    return BoxLengths;

  } else if (checkGradient(value)) {
    return Gradient;

  } else if (checkIcon(value)) {
    return Icon;

  } else if (checkNumber(value)) {
    return Number;

  } else if (checkOutline(value)) {
    return Outline;

  } else if (checkOrigin(value)) {
    return Origin;

  } else if (checkOutlineStyle(value)) {
    return OutlineStyle;

  } else if (checkOutlineRadius(value)) {
    return OutlineRadius;

  } else if (checkPaletteRole(value)) {
    return PaletteRole;

  } else if (checkRepeat(value)) {
    return Repeat;

  } else if (checkUrl(value)) {
    return Url;

  } else if (checkPosition(value)) {
    return Position;

  } else if (checkTextDecoration(value)) {
    return TextDecoration;

  } else if (checkStylesheetEdit(value)) {
    return StylesheetEditGood;

  } else if (checkStylesheetEditBad(value)) {
    return StylesheetEditBad;
  }

  return NoAttributeValue;
}

QMap<int, QString>
DataStore::fuzzyTestBrush(const QString& value)
{
  QMap<int, QString> data;
  data.insert(fuzzySearch(value, m_colors));
  data.insert(fuzzySearch(value, m_paletteRoles));
  data.insert(fuzzySearch(value, m_gradient));
  return data;
}

QMap<QString, QStringList>
DataStore::initialiseSubControlMap()
{
  QMap<QString, QStringList> map;
  map.insert("add-line", addControls(1, new QString("QScrollBar")));
  map.insert("add-page", addControls(1, new QString("QScrollBar")));
  map.insert("branch", addControls(1, new QString("QTreeBar")));
  map.insert("chunk", addControls(1, new QString("QProgressBar")));
  map.insert(
    "close-button ",
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

QStringList
DataStore::initialiseWidgetList()
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

  //  QString fuzzyWidgets = m_widgets.first();
  //  for (int i = 1; i < m_widgets.size(); i++) {
  //    fuzzyWidgets += " ";
  //    fuzzyWidgets += m_widgets.at(i);
  //  }
  //  char *pline = new char[fuzzyWidgets.size() + 1];
  //  strcpy ( pline, fuzzyWidgets.toStdString().c_str() );
  //  m_fuzzyWidgets = pline;

  return list;
}

QStringList
DataStore::initialisePseudoStateList()
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

QStringList
DataStore::initialisePropertyList()
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

QStringList
DataStore::initialiseStylesheetProperties()
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

QMap<QString, AttributeType>
DataStore::initialiseStylesheetMap()
{
  QMap<QString, AttributeType> map;
  map.insert("widget", StylesheetEditGood);
  map.insert("subcontrol", StylesheetEditGood);
  map.insert("pseudostate", StylesheetEditGood);
  map.insert("subcontrolmarker", StylesheetEditGood);
  map.insert("pseudostatemarker", StylesheetEditGood);
  map.insert("property", StylesheetEditGood);
  map.insert("propertymarker", StylesheetEditGood);
  map.insert("value", StylesheetEditGood);
  map.insert("startbrace", StylesheetEditGood);
  map.insert("endbrace", StylesheetEditGood);
  map.insert("bracematch", StylesheetEditGood);
  map.insert("comment", StylesheetEditGood);
  map.insert("bad", StylesheetEditBad);
  // TODO more values
  return map;
}

QStringList
DataStore::initialiseColorList()
{
  QStringList list;
  list << "black"
       << "silver"
       << "gray"
       << "whitesmoke"
       << "maroon"
       << "red"
       << "purple"
       << "fuchsia"
       << "green"
       << "lime"
       << "olivedrab"
       << "yellow"
       << "navy"
       << "blue"
       << "teal"
       << "aquamarine"
       << "orange"
       << "aliceblue"
       << "antiquewhite"
       << "aqua"
       << "azure"
       << "beige"
       << "bisque"
       << "blanchedalmond"
       << "blueviolet"
       << "brown"
       << "burlywood"
       << "cadetblue"
       << "chartreuse"
       << "chocolate"
       << "coral"
       << "cornflowerblue"
       << "cornsilk"
       << "crimson"
       << "darkblue"
       << "darkcyan"
       << "darkgoldenrod"
       << "darkgray"
       << "darkgreen"
       << "darkgrey"
       << "darkkhaki"
       << "darkmagenta"
       << "darkolivegreen"
       << "darkorange"
       << "darkorchid"
       << "darkred"
       << "darksalmon"
       << "darkseagreen"
       << "darkslateblue"
       << "darkslategray"
       << "darkslategrey"
       << "darkturquoise"
       << "darkviolet"
       << "deeppink"
       << "deepskyblue"
       << "dimgray"
       << "dimgrey"
       << "dodgerblue"
       << "firebrick"
       << "floralwhite"
       << "forestgreen"
       << "gainsboro"
       << "ghostwhite"
       << "goldenrod"
       << "gold"
       << "greenyellow"
       << "grey"
       << "honeydew"
       << "hotpink"
       << "indianred"
       << "indigo"
       << "ivory"
       << "khaki"
       << "lavenderblush"
       << "lavender"
       << "lawngreen"
       << "lemonchiffon"
       << "lightblue"
       << "lightcoral"
       << "lightcyan"
       << "lightgoldenrodyellow"
       << "lightgray"
       << "lightgreen"
       << "lightgrey"
       << "lightpink"
       << "lightsalmon"
       << "lightseagreen"
       << "lightskyblue"
       << "lightslategray"
       << "lightslategrey"
       << "lightsteelblue"
       << "lightyellow"
       << "limegreen"
       << "linen"
       << "mediumaquamarine"
       << "mediumblue"
       << "mediumorchid"
       << "mediumpurple"
       << "mediumseagreen"
       << "mediumslateblue"
       << "mediumspringgreen"
       << "mediumturquoise"
       << "mediumvioletred"
       << "midnightblue"
       << "mintcream"
       << "mistyrose"
       << "moccasin"
       << "navajowhite"
       << "oldlace"
       << "olive"
       << "orangered"
       << "orchid"
       << "palegoldenrod"
       << "palegreen"
       << "paleturquoise"
       << "palevioletred"
       << "papayawhip"
       << "peachpuff"
       << "peru"
       << "pink"
       << "plum"
       << "powderblue"
       << "rosybrown"
       << "royalblue"
       << "saddlebrown"
       << "salmon"
       << "sandybrown"
       << "seagreen"
       << "seashell"
       << "sienna"
       << "skyblue"
       << "slateblue"
       << "slategray"
       << "slategrey"
       << "snow"
       << "springgreen"
       << "steelblue"
       << "tan"
       << "thistle"
       << "tomato"
       << "transparent"
       << "turquoise"
       << "violet"
       << "wheat"
       << "white"
       << "yellowgreen"
       << "rebeccapurple";
  return list;
}

QMap<QString, AttributeType>
DataStore::initialiseAttributeMap()
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
  map.insert("widget", StylesheetEditGood);
  map.insert("subcontrol", StylesheetEditGood);
  map.insert("pseudostate", StylesheetEditGood);
  map.insert("subcontrolmarker", StylesheetEditGood);
  map.insert("pseudostatemarker", StylesheetEditGood);
  map.insert("property", StylesheetEditGood);
  map.insert("propertymarker", StylesheetEditGood);
  map.insert("value", StylesheetEditGood);
  map.insert("startbrace", StylesheetEditGood);
  map.insert("endbrace", StylesheetEditGood);
  map.insert("bracematch", StylesheetEditGood);
  map.insert("comment", StylesheetEditGood);
  map.insert("bad", StylesheetEditBad);

  return map;
}

QStringList
DataStore::addControls(int count, ...)
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
