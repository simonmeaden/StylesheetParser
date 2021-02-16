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
//#include "safe_lib.h"
#include "string"
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
  , m_braceCount(0)
  , m_manualMove(false)
  , m_hasSuggestion(false)
  , m_maxSuggestionCount(30)
{
  initialiseWidgetModel();
}

DataStore::~DataStore()
{
  emit finished();
}

void
DataStore::addWidget(const QString& widget, const QString& parent)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->addWidget(widget, parent);
}

void
DataStore::removeWidget(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->removeWidget(name);
}

bool
DataStore::containsWidget(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  // NOT toLower() as widget names are cased.
  return m_widgetModel->hasWidget(name);
}

QMultiMap<int, QString>
DataStore::fuzzySearchWidgets(const QString& name)
{
  return m_widgetModel->fuzzySearchWidgets(name);
}

bool
DataStore::containsProperty(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->containsProperty(name.toLower());
}

QMultiMap<int, QString>
DataStore::fuzzySearchProperty(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return fuzzySearchProperty(name);
}

QMultiMap<int, QString>
DataStore::fuzzySearchPropertyValue(const QString& name, const QString& value)
{
  return m_widgetModel->fuzzySearchPropertyValue(name, value);
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
  return m_widgetModel->containsPseudoState(name.toLower());
}

QMultiMap<int, QString>
DataStore::fuzzySearchPseudoStates(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->fuzzySearchPseudoStates(name);
}

bool
DataStore::containsSubControl(const QString& name)
{
  return m_widgetModel->containsSubControl(name);
}

bool
DataStore::isValidSubControlForWidget(const QString& widget,
                                      const QString& subcontrol)
{
  return m_widgetModel->isValidSubControlForWidget(widget, subcontrol);
}

QMultiMap<int, QString>
DataStore::fuzzySearchSubControl(const QString& name)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->fuzzySearchSubControl(name);
}

bool
DataStore::checkSubControlForWidget(const QString& widget, const QString& name)
{
  return m_widgetModel->checkSubControlForWidget(widget, name);
}

bool
WidgetModel::checkAlignment(const QString& value) const
{
  return m_alignmentValues.contains(value);
}

bool
WidgetModel::checkAttachment(const QString& value) const
{
  return m_attachment.contains(value);
}

bool
WidgetModel::checkBackground(const QString& value) const
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
WidgetModel::checkBool(const QString& value) const
{
  if (value == "true" || value == "false") {
    return true;
  }

  return false;
}

bool
WidgetModel::checkBoolean(const QString& value) const
{
  if (value == "0" || value == "1") {
    return true;
  }

  return false;
}

bool
WidgetModel::checkBorder(const QString& value) const
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
WidgetModel::checkBorderImage(const QString& value) const
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
WidgetModel::checkBorderStyle(const QString& value) const
{
  return m_borderStyle.contains(value);
}

bool
WidgetModel::checkBoxColors(const QString& value) const
{
  return checkColor(value);
}

bool
WidgetModel::checkBoxLengths(const QString& value) const
{
  return checkLength(value);
}

bool
WidgetModel::checkBrush(const QString& value) const
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
WidgetModel::checkColor(const QString& value) const
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
WidgetModel::checkFontStyle(const QString& value) const
{
  return m_fontStyle.contains(value);
}

bool
WidgetModel::checkFont(const QString& value) const
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
WidgetModel::checkFontSize(const QString& value) const
{
  return checkLength(value);
}

bool
WidgetModel::checkFontWeight(const QString& value) const
{
  return m_fontWeight.contains(value);
}

bool
WidgetModel::checkGradient(const QString& value) const
{
  return m_gradient.contains(value);
}

bool
WidgetModel::checkIcon(const QString& value) const
{
  if (checkUrl(value)) {
    return true;
  }

  return m_icon.contains(value);
}

bool
WidgetModel::checkLength(const QString& value) const
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
WidgetModel::checkNumber(const QString& value) const
{
  bool ok = false;
  // don't actually need the value, I only want to know
  // if it actually is a number.
  value.toDouble(&ok);
  return ok;
}

bool
WidgetModel::checkOutline(const QString& value) const
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
WidgetModel::checkOrigin(const QString& value) const
{
  return m_origin.contains(value);
}

bool
WidgetModel::checkOutlineStyle(const QString& value) const
{
  return m_outlineStyle.contains(value);
}

bool
WidgetModel::checkOutlineColor(const QString& value) const
{
  if (value == m_outlineColor) {
    return true;
  }

  return checkColor(value);
}

bool
WidgetModel::checkOutlineWidth(const QString& value) const
{
  if (m_outlineWidth.contains(value)) {
    return true;
  }

  return checkLength(value);
}

bool
WidgetModel::checkOutlineOffset(const QString& value) const
{
  return checkLength(value);
}

bool
WidgetModel::checkOutlineRadius(const QString& value) const
{
  // check for 4 radius values.
  return checkRadius(value);
}

bool
WidgetModel::checkPaletteRole(const QString& value) const
{
  if (m_paletteRoles.contains(value)) {
    return true;
  }

  return false;
}

bool
WidgetModel::checkRadius(const QString& value) const
{
  return checkLength(value);
}

bool
WidgetModel::checkRepeat(const QString& value) const
{
  return m_repeat.contains(value);
}

bool
WidgetModel::checkUrl(const QString& value) const
{
  //  QFile file(value);

  //  if (file.exists()) {
  //    return true;
  //  }
  QString v = value.trimmed();
  if (v.trimmed().startsWith("url")) {
    v = v.mid(3).trimmed();
  } else {
    return false;
  }
  if (v.startsWith("(")) {
    v = v.mid(1).trimmed();
  } else {
    return false;
  }
  if (v.endsWith(")")) {
    v = v.mid(0, v.length() - 1).trimmed();
  } else {
    return false;
  }
  QUrl url(v);
  return url.isValid();
}

bool
WidgetModel::checkPosition(const QString& value) const
{
  return m_position.contains(value);
}

bool
WidgetModel::checkTextDecoration(const QString& value) const
{
  return m_textDecoration.contains(value);
}

bool
WidgetModel::checkStylesheetEdit(const QString& value,
                                 StylesheetData* data) const
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
WidgetModel::checkStylesheetEditBad(const QString& value,
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
WidgetModel::checkStylesheetFontWeight(const QString& value,
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

void
DataStore::initialiseWidgetModel()
{
  m_widgetModel = new WidgetModel();
}

bool
WidgetModel::checkPropertyValue(AttributeType propertyAttribute,
                                const QString& valuename)
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

      // TODO add custom property value checks.
      //    case StylesheetEditGood:
      //      return checkStylesheetEdit(valuename, data);

      //    case StylesheetEditBad:
      //      return checkStylesheetEditBad(valuename, data);
  }

  return false;
}

// bool
// DataStore::ifValidStylesheetValue(const QString& propertyname,
//                                  const QString& valuename,
//                                  StylesheetData* data)
//{
//  QMutexLocker locker(&m_mutex);
//  return m_widgetModel->ifValidStylesheetValue(propertyname, valuename, data);
//}

bool
DataStore::isValidPropertyValueForProperty(const QString& propertyname,
                                           const QString& valuename)
{
  return m_widgetModel->isValidPropertyValueForProperty(propertyname,
                                                        valuename);
}

QStringList
DataStore::possibleWidgetsForSubControl(const QString& name)
{
  return m_widgetModel->possibleWidgetsForSubControl(name);
}

QStringList
DataStore::possibleSubControlsForWidget(const QString& widget)
{
  return m_widgetModel->possibleSubControlsForWidget(widget);
}

void
DataStore::addSubControl(const QString& control, const QString& widget)
{
  QMutexLocker locker(&m_mutex);
  QStringList widgets;
  widgets << widget;
  m_widgetModel->addSubControl(control, widgets);
}

void
DataStore::addSubControl(const QString& control, QStringList& widgets)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->addSubControl(control, widgets);
}

void
DataStore::removeSubControl(const QString& control)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->removeSubControl(control.trimmed());
}

void
DataStore::addPseudoState(const QString& state)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->addPseudoState(state, true);
}

void
DataStore::removePseudoState(const QString& state)
{
  QMutexLocker locker(&m_mutex);
  m_widgetModel->removePseudoState(state);
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

// bool
// DataStore::isBraceCountZero()
//{
//  return m_braceStack.isEmpty();
//}

// void
// DataStore::pushStartBrace(StartBraceNode* startbrace)
//{
//  m_startbraces.append(startbrace);
//  m_braceStack.push(startbrace);
//}

// void
// DataStore::pushEndBrace(EndBraceNode* endbrace)
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

QIcon
DataStore::addDColonIcon() const
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

QIcon
DataStore::badColonIcon() const
{
  return m_badColonIcon;
}

QIcon
DataStore::badDColonIcon() const
{
  return m_badDColonIcon;
}

QIcon
DataStore::noIcon() const
{
  return m_noIcon;
}

QIcon
DataStore::fuzzyIcon() const
{
  return m_fuzzyIcon;
}

AttributeType
DataStore::propertyValueAttribute(const QString& value)
{
  QMutexLocker locker(&m_mutex);
  return m_widgetModel->propertyValueAttribute(value);
}

// QMap<QString, AttributeType>
// DataStore::initialiseStylesheetMap()
//{
//  QMap<QString, AttributeType> map;
//  map.insert("widget", StylesheetEditGood);
//  map.insert("subcontrol", StylesheetEditGood);
//  map.insert("pseudostate", StylesheetEditGood);
//  map.insert("subcontrolmarker", StylesheetEditGood);
//  map.insert("pseudostatemarker", StylesheetEditGood);
//  map.insert("property", StylesheetEditGood);
//  map.insert("propertymarker", StylesheetEditGood);
//  map.insert("value", StylesheetEditGood);
//  map.insert("startbrace", StylesheetEditGood);
//  map.insert("endbrace", StylesheetEditGood);
//  map.insert("bracematch", StylesheetEditGood);
//  map.insert("comment", StylesheetEditGood);
//  map.insert("bad", StylesheetEditBad);
//  // TODO more values
//  return map;
//}

WidgetItem::WidgetItem(const QString& name, WidgetItem* parent)
  : m_name(name)
  , m_parent(parent)
{}

QString
WidgetItem::name()
{
  return m_name;
}

WidgetItem*
WidgetItem::parent()
{
  return m_parent;
}

QList<WidgetItem*>
WidgetItem::children()
{
  return m_children;
}

void
WidgetItem::addChild(WidgetItem* child)
{
  m_children.append(child);
}

void
WidgetItem::removeChild(const QString& name)
{
  for (auto child : m_children) {
    if (child->name() == name) {
      m_children.removeOne(child);
      break;
    }
  }
}

bool
WidgetItem::hasChildren()
{
  return m_children.isEmpty();
}

bool
WidgetItem::isExtraWidget() const
{
  return m_extraWidget;
}

void
WidgetItem::setExtraWidget(bool extraWidget)
{
  m_extraWidget = extraWidget;
}

void
WidgetItem::addSubcontrol(const QString& control)
{
  if (!m_subcontrols.contains(control))
    m_subcontrols.append(control);
}

bool
WidgetItem::removeSubcontrol(const QString& control)
{
  return m_subcontrols.removeOne(control);
}

bool
WidgetItem::hasSubControl(const QString& name)
{
  return m_subcontrols.contains(name);
}

QStringList
WidgetItem::subControls()
{
  return m_subcontrols;
}

void
WidgetModel::initPaletteRoles()
{
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
}

void
WidgetModel::initColorNames()
{
  m_colors << "black"
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
}

void
WidgetModel::initProperties()
{
  addProperty("alternate-background-color");
  addProperty("background");
  addProperty("background-color");
  addProperty("background-image");
  addProperty("background-repeat");
  addProperty("background-position");
  addProperty("background-attachment");
  addProperty("background-clip");
  addProperty("background-origin");
  addProperty("border");
  addProperty("border-top");
  addProperty("border-right");
  addProperty("border-bottom");
  addProperty("border-left");
  addProperty("border-color");
  addProperty("border-top-color");
  addProperty("border-right-color");
  addProperty("border-bottom-color");
  addProperty("border-left-color");
  addProperty("border-image");
  addProperty("border-radius");
  addProperty("border-top-left-radius");
  addProperty("border-top-right-radius");
  addProperty("border-bottom-right-radius");
  addProperty("border-bottom-left-radius");
  addProperty("border-style");
  addProperty("border-top-style");
  addProperty("border-right-style");
  addProperty("border-bottom-style");
  addProperty("border-left-style");
  addProperty("border-width");
  addProperty("border-top-width");
  addProperty("border-right-width");
  addProperty("border-bottom-width");
  addProperty("border-left-width");
  addProperty("bottom");
  addProperty("button-laout");
  addProperty("color");
  addProperty("dialogbuttonbox-buttons-have-icons");
  addProperty("font");
  addProperty("font-family");
  addProperty("font-size");
  addProperty("font-style");
  addProperty("font-weight");
  addProperty("gridline-color");
  addProperty("height");
  addProperty("icon");
  addProperty("icon-size");
  addProperty("image");
  addProperty("image-position");
  addProperty("left");
  addProperty("lineedit-password-character");
  addProperty("lineedit-password-mask-delay");
  addProperty("margin");
  addProperty("margin-top");
  addProperty("margin-right");
  addProperty("margin-bottom");
  addProperty("margin-left");
  addProperty("max-height");
  addProperty("max-width");
  addProperty("messagebox-text-interaction-flags");
  addProperty("min-height");
  addProperty("min-width");
  addProperty("opacity");
  addProperty("outline");
  addProperty("outline-color");
  addProperty("outline-offset");
  addProperty("outline-style");
  addProperty("outline-radius");
  addProperty("outline-bottom-left-radius");
  addProperty("outline-bottom-right-radius");
  addProperty("outline-top-left-radius");
  addProperty("outline-top-right-radius");
  addProperty("padding");
  addProperty("padding-top");
  addProperty("padding-right");
  addProperty("padding-bottom");
  addProperty("padding-left");
  addProperty("paint-alternating-row-colors-for-empty-area");
  addProperty("position");
  addProperty("right");
  addProperty("selection-background-color");
  addProperty("selection-color");
  addProperty("show-decoration-selected");
  addProperty("spacing");
  addProperty("subcontrol-origin");
  addProperty("subcontrol-position");
  addProperty("titlebar-show-tooltips-on-buttons");
  addProperty("widget-animation-duration");
  addProperty("text-align");
  addProperty("text-decoration");
  addProperty("top");
  addProperty("width");
  addProperty("-qt-background-role");
  addProperty("-qt-style-features");
  // I might as well add stylesheet stuff for this widget.
  addProperty("widget", true);
  addProperty("subcontrol", true);
  addProperty("subcontrolmarker", true);
  addProperty("pseudostate", true);
  addProperty("pseudostatemarker", true);
  addProperty("property", true);
  addProperty("propertymarker", true);
  addProperty("value", true);
  addProperty("startbrace", true);
  addProperty("endbrace", true);
  addProperty("bracematch", true);
  addProperty("comment", true);
  addProperty("bad", true);
}

void
WidgetModel::initSubControls()
{
  addSubControl("add-line", addControls(1, new QString("QScrollBar")));
  addSubControl("add-page", addControls(1, new QString("QScrollBar")));
  addSubControl("branch", addControls(1, new QString("QTreeView")));
  addSubControl("chunk", addControls(1, new QString("QProgressBar")));
  addSubControl(
    "close-button",
    addControls(2, new QString("QDockWidget"), new QString("QTabBar")));
  addSubControl("corner", addControls(1, new QString("QAbstractScrollArea")));
  addSubControl("down-arrow",
                addControls(4,
                            new QString("QComboBox"),
                            new QString("QHeaderView"),
                            new QString("QScrollBar"),
                            new QString("QSpinBox")));
  addSubControl(
    "down-button",
    addControls(2, new QString("QScrollBar"), new QString("QSpinBox")));
  addSubControl("drop-down", addControls(1, new QString("QComboBox")));
  addSubControl("float-button", addControls(1, new QString("QDockWidget")));
  addSubControl("groove", addControls(1, new QString("QSlider")));
  addSubControl("indicator",
                addControls(5,
                            new QString("QAbstractItemView"),
                            new QString("QCheckBox"),
                            new QString("QRadioButton"),
                            new QString("QMenu"),
                            new QString("QGroupBox")));
  addSubControl("handle",
                addControls(3,
                            new QString("QScrollBar"),
                            new QString("QSplitter"),
                            new QString("QSlider")));
  addSubControl(
    "icon",
    addControls(2, new QString("QAbstractItemView"), new QString("QMenu")));
  addSubControl("item",
                addControls(4,
                            new QString("QAbstractItemView"),
                            new QString("QMenuBar"),
                            new QString("QMenu"),
                            new QString("QStatusBar")));
  addSubControl("left-arrow", addControls(1, new QString("QScrollBar")));
  addSubControl("left-corner", addControls(1, new QString("QTabWidget")));
  addSubControl("menu-arrow", addControls(1, new QString("QToolButton")));
  addSubControl("menu-button", addControls(1, new QString("QToolButton")));
  addSubControl("menu-indicator", addControls(1, new QString("QPushButton")));
  addSubControl(
    "right-arrow",
    addControls(2, new QString("QMenu"), new QString("QScrollBar")));
  addSubControl("pane", addControls(1, new QString("QTabWidget")));
  addSubControl("right-corner", addControls(1, new QString("QTabWidget")));
  addSubControl("scroller",
                addControls(2, new QString("QMenu"), new QString("QTabBar")));
  addSubControl("section", addControls(1, new QString("QHeaderView")));
  addSubControl(
    "separator",
    addControls(2, new QString("QMenu"), new QString("QMainWindow")));
  addSubControl("sub-line", addControls(1, new QString("QScrollBar")));
  addSubControl("sub-page", addControls(1, new QString("QScrollBar")));
  addSubControl(
    "tab", addControls(2, new QString("QTabBar"), new QString("QToolBox")));
  addSubControl("tab-bar", addControls(1, new QString("QTabWidget")));
  addSubControl("tear", addControls(1, new QString("QTabBar")));
  addSubControl("tearoff", addControls(1, new QString("QMenu")));
  addSubControl("text", addControls(1, new QString("QAbstractItemView")));
  addSubControl(
    "title",
    addControls(2, new QString("QGroupBox"), new QString("QDockWidget")));
  addSubControl("up-arrow",
                addControls(3,
                            new QString("QHeaderView"),
                            new QString("QScrollBar"),
                            new QString("QSpinBox")));
  addSubControl("up-button", addControls(1, new QString("QSpinBox")));
}

void
WidgetModel::initPseudoStates()
{
  addPseudoState("active");
  addPseudoState("adjoins-item");
  addPseudoState("alternate");
  addPseudoState("bottom");
  addPseudoState("checked");
  addPseudoState("closable");
  addPseudoState("closed");
  addPseudoState("default");
  addPseudoState("disabled");
  addPseudoState("editable");
  addPseudoState("edit-focus");
  addPseudoState("enabled");
  addPseudoState("exclusive");
  addPseudoState("first");
  addPseudoState("flat");
  addPseudoState("floatable");
  addPseudoState("focus");
  addPseudoState("has-children");
  addPseudoState("has-siblings");
  addPseudoState("horizontal");
  addPseudoState("hover");
  addPseudoState("indeterminate");
  addPseudoState("last");
  addPseudoState("left");
  addPseudoState("maximized");
  addPseudoState("middle");
  addPseudoState("minimized");
  addPseudoState("movable");
  addPseudoState("no-frame");
  addPseudoState("non-exclusive");
  addPseudoState("off");
  addPseudoState("on");
  addPseudoState("only-one");
  addPseudoState("open");
  addPseudoState("next-selected");
  addPseudoState("pressed");
  addPseudoState("previous-selected");
  addPseudoState("read-only");
  addPseudoState("right");
  addPseudoState("selected");
  addPseudoState("top");
  addPseudoState("unchecked");
  addPseudoState("vertical");
  addPseudoState("window");
}

void
WidgetModel::initWidgetTree()
{
  m_root = new WidgetItem("QWidget", nullptr);
  m_widgets.insert("QWidget", m_root);

  addWidget("QWidget", "QAbstractButton");
  addWidget("QAbstractButton", "QPushButton");
  addWidget("QAbstractButton", "QCheckBox");
  addWidget("QAbstractButton", "QRadioButton");
  addWidget("QAbstractButton", "QToolButton");
  addWidget("QWidget", "QAbstractSlider");
  addWidget("QAbstractSlider", "QSlider");
  addWidget("QAbstractSlider", "QDial");
  addWidget("QAbstractSlider", "QScrollBar");
  addWidget("QWidget", "QAbstractSpinBox");
  addWidget("QAbstractSpinBox", "QSpinBox");
  addWidget("QAbstractSpinBox", "QDoubleSpinBox");
  addWidget("QAbstractSpinBox", "QDateTimeEdit");
  addWidget("QDateTimeEdit", "QDateEdit");
  addWidget("QDateTimeEdit", "QTimeEdit");
  addWidget("QWidget", "QDialog");
  addWidget("QWidget", "QComboBox");
  addWidget("QComboBox", "QFontComboBox");
  addWidget("QDialog", "QInputDialog");
  addWidget("QDialog", "QMessageBox");
  addWidget("QDialog", "QColorDialog");
  addWidget("QDialog", "QErrorMessage");
  addWidget("QDialog", "QFileDialog");
  addWidget("QDialog", "QFontDialog");
  addWidget("QDialog", "QProgressDialog");
  addWidget("QWidget", "QDialogButtonBox");
  addWidget("QWidget", "QFrame");
  addWidget("QFrame", "QAbstractScrollArea");
  addWidget("QAbstractScrollArea", "QAbstractItemView");
  addWidget("QAbstractScrollArea", "QGraphicsView");
  addWidget("QAbstractScrollArea", "QMdiArea");
  addWidget("QAbstractScrollArea", "QScrollArea");
  addWidget("QAbstractItemView", "QColumnView");
  addWidget("QAbstractItemView", "QHeaderView");
  addWidget("QAbstractItemView", "QListView");
  addWidget("QAbstractItemView", "QListWidget");
  addWidget("QAbstractItemView", "QUndoView");
  addWidget("QAbstractItemView", "QTreeView");
  addWidget("QTreeView", "QTreeWidget");
  addWidget("QAbstractItemView", "QTableView");
  addWidget("QTableView", "QTableWidget");
  addWidget("QAbstractScrollArea", "QPlainTextEdit");
  addWidget("QAbstractScrollArea", "QTextEdit");
  addWidget("QFrame", "QLabel");
  addWidget("QFrame", "QLCDNumber");
  addWidget("QFrame", "QSplitter");
  addWidget("QFrame", "QStackedWidget");
  addWidget("QFrame", "QToolBox");
  addWidget("QWidget", "QGroupBox");
  addWidget("QWidget", "QLineEdit");
  addWidget("QWidget", "QMainWindow");
  addWidget("QWidget", "QDockWidget");
  addWidget("QWidget", "QFocusFrame");
  addWidget("QWidget", "QKeySequenceEdit");
  addWidget("QWidget", "QMdiSubWindow");
  addWidget("QWidget", "QOpenGLWidget");
  addWidget("QWidget", "QProgressBar");
  addWidget("QWidget", "QQuickWidget");
  addWidget("QWidget", "QSizeGrip");
  addWidget("QWidget", "QSplashScreen");
  addWidget("QWidget", "QSplitterHandle");
  addWidget("QWidget", "QSvgWidget");
  addWidget("QWidget", "QWizardPage");
  //  addWidget("QWidget", "QDesktopWidget"); // Obsolete
  addWidget("QWidget", "QMenu");
  addWidget("QWidget", "QMenuBar");
  addWidget("QWidget", "QRubberBand");
  addWidget("QWidget", "QStatusBar");
  addWidget("QWidget", "QTabBar");
  addWidget("QWidget", "QTabWidget");
  addWidget("QWidget", "QToolBar");
  addWidget("QWidget", "QCalendarWidget");
  //  // extra widgets.
  //  addWidget("QPlainTextEdit", "StylesheetEdit", true);
}

void
WidgetModel::initAlignment()
{
  m_alignmentValues << "top"
                    << "bottom"
                    << "left"
                    << "right"
                    << "center";
}

void
WidgetModel::initGradients()
{
  m_gradient << "qlineargradient"
             << "qradialgradient"
             << "qconicalgradient";
}

void
WidgetModel::initAttachments()
{
  m_attachment << "scroll"
               << "fixed";
}

void
WidgetModel::initBorderStyle()
{
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
}

void
WidgetModel::initBorderImage()
{
  m_borderImage << "stretch"
                << "repeat";
}

void
WidgetModel::initFontStyle()
{
  m_fontStyle << "normal"
              << "italic"
              << "oblique";
}

void
WidgetModel::initFontWeight()
{
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
}

void
WidgetModel::initIcon()
{
  m_icon << "disabled"
         << "active"
         << "normal"
         << "selected"
         << "on"
         << "off";
}

void
WidgetModel::initOrigin()
{
  m_origin << "dotted"
           << "solid"
           << "double"
           << "groove"
           << "ridge"
           << "inset"
           << "outset"
           << "none"
           << "hidden";
}

void
WidgetModel::initOutlineStyle()
{
  m_outlineStyle << "dotted"
                 << "solid"
                 << "double"
                 << "groove"
                 << "ridge"
                 << "inset"
                 << "outset"
                 << "none"
                 << "hidden";
}

void
WidgetModel::initOutlineColor()
{
  m_outlineColor = "invert";
}

void
WidgetModel::initOutlineWidth()
{
  m_outlineWidth << "thin"
                 << "medium"
                 << "thick";
}

void
WidgetModel::initPosition()
{
  m_position << "relative"
             << "absolute";
}

void
WidgetModel::initRepeat()
{
  m_repeat << "repeat-x"
           << "repeat-y"
           << "repeat"
           << "no-repeat";
}

void
WidgetModel::initTextDecoration()
{
  m_textDecoration << "none"
                   << "underline"
                   << "overline"
                   << "line-through";
}

void
WidgetModel::initAttributeMap()
{
  m_attributes.insert("alternate-background-color", Brush);
  m_attributes.insert("background", Background);
  m_attributes.insert("background-color", Brush);
  m_attributes.insert("background-image", Url);
  m_attributes.insert("background-repeat", Repeat);
  m_attributes.insert("background-position", Alignment);
  m_attributes.insert("background-clip", Origin);
  m_attributes.insert("background-origin", Origin);
  m_attributes.insert("border", Border);
  m_attributes.insert("border-top", Border);
  m_attributes.insert("border-left", Border);
  m_attributes.insert("border-right", Border);
  m_attributes.insert("border-bottom", Border);
  m_attributes.insert("border-color", BoxColors);
  m_attributes.insert("border-top-color", Brush);
  m_attributes.insert("border-right-color", Brush);
  m_attributes.insert("border-bottom-color", Brush);
  m_attributes.insert("border-left-color", Brush);
  m_attributes.insert("border-image", BorderImage);
  m_attributes.insert("border-radius", Radius);
  m_attributes.insert("border-top-left-radius", Radius);
  m_attributes.insert("border-top-right-radius", Radius);
  m_attributes.insert("border-bottom-right-radius", Radius);
  m_attributes.insert("border-bottom-left-radius", Radius);
  m_attributes.insert("border-style", BorderStyle);
  m_attributes.insert("border-top-style", BorderStyle);
  m_attributes.insert("border-right-style", BorderStyle);
  m_attributes.insert("border-bottom-style", BorderStyle);
  m_attributes.insert("border-left-style", BorderStyle);
  m_attributes.insert("border-width", BoxLengths);
  m_attributes.insert("border-top-width", Length);
  m_attributes.insert("border-right-width", Length);
  m_attributes.insert("border-bottom-width", Length);
  m_attributes.insert("border-left-width", Length);
  m_attributes.insert("bottom", Length);
  m_attributes.insert("button-layout", Number);
  m_attributes.insert("color", Brush);
  m_attributes.insert("dialogbuttonbox-buttons-have-icons", Brush);
  m_attributes.insert("font", Font);
  m_attributes.insert("font-family", String);
  m_attributes.insert("font-size", FontSize);
  m_attributes.insert("font-style", FontStyle);
  m_attributes.insert("font-weight", FontWeight);
  m_attributes.insert("gridline-color", Color);
  m_attributes.insert("height", Length);
  m_attributes.insert("icon", Url);
  m_attributes.insert("icon-size", Length);
  m_attributes.insert("image", Url);
  m_attributes.insert("image-position", Alignment);
  m_attributes.insert("left", Length);
  m_attributes.insert("lineedit-password-character", Number);
  m_attributes.insert("lineedit-password-mask-delay", Number);
  m_attributes.insert("margin", BoxLengths);
  m_attributes.insert("margin-top", Length);
  m_attributes.insert("margin-right", Length);
  m_attributes.insert("margin-bottom", Length);
  m_attributes.insert("margin-left", Length);
  m_attributes.insert("max-height", Length);
  m_attributes.insert("max-width", Length);
  m_attributes.insert("messagebox-text-interaction-flags", Number);
  m_attributes.insert("min-height", Length);
  m_attributes.insert("min-width", Length);
  m_attributes.insert("opacity", Length);
  m_attributes.insert("outline", Outline); // not known??
  m_attributes.insert("outline-color", Color);
  m_attributes.insert("outline-offset", Length);
  m_attributes.insert("outline-style", OutlineStyle);
  m_attributes.insert("outline-radius", OutlineRadius);
  m_attributes.insert("outline-bottom-left-radius", Radius);
  m_attributes.insert("outline-bottom-right-radius", Radius);
  m_attributes.insert("outline-top-left-radius", Radius);
  m_attributes.insert("outline-top-right-radius", Radius);
  m_attributes.insert("padding", BoxLengths);
  m_attributes.insert("padding-top", Length);
  m_attributes.insert("padding-right", Length);
  m_attributes.insert("padding-bottom", Length);
  m_attributes.insert("padding-left", Length);
  m_attributes.insert("paint-alternating-row-colors-for-empty-area", Bool);
  m_attributes.insert("position", Position);
  m_attributes.insert("right", Length);
  m_attributes.insert("selection-background-color", Brush);
  m_attributes.insert("selection-color", Brush);
  m_attributes.insert("show-decoration-selected", Boolean);
  m_attributes.insert("spacing", Length);
  m_attributes.insert("subcontrol-position", Alignment);
  m_attributes.insert("titlebar-show-tooltips-on-buttons", Bool);
  m_attributes.insert("widget-animation-duration", Number);
  m_attributes.insert("text-align", Alignment);
  m_attributes.insert("text-decoration", TextDecoration);
  m_attributes.insert("top", Length);
  m_attributes.insert("width", Length);
  m_attributes.insert("-qt-background-role", PaletteRole);
  m_attributes.insert("-qt-style-features", List);
}

WidgetModel::WidgetModel()
{
  initWidgetTree();
  initSubControls();
  initPseudoStates();
  initProperties();
  initColorNames();
  initPaletteRoles();
  initAlignment();
  initGradients();
  initAttachments();
  initBorderStyle();
  initBorderImage();
  initFontStyle();
  initFontWeight();
  initIcon();
  initOrigin();
  initOutlineStyle();
  initOutlineColor();
  initOutlineWidth();
  initPosition();
  initRepeat();
  initTextDecoration();
  initAttributeMap();
}

/*!
   \brief Takes a variable size list of strings add creates a QStringList odf
   them.
 */
QStringList
WidgetModel::addControls(int count, ...)
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

void
WidgetModel::addWidget(const QString& parentName,
                       const QString& name,
                       bool extraWidget)
{
  WidgetItem* parent = m_widgets.value(parentName);
  if (parent) {
    if (!m_widgets.contains(name)) {
      WidgetItem* item = new WidgetItem(name, parent);
      item->setExtraWidget(extraWidget);
      parent->addChild(item);
      m_widgets.insert(name, item);
    }
  }
}

void
WidgetModel::removeWidget(const QString& name)
{
  WidgetItem* parent = m_widgets.value(name)->parent();
  if (parent->isExtraWidget()) {
    m_widgets.remove(name);
    parent->removeChild(name);
  }
}

bool
WidgetModel::hasWidget(const QString& widget)
{
  return m_widgets.contains(widget);
}

WidgetItem*
WidgetModel::widgetItem(const QString& name)
{
  return m_widgets.value(name);
}

void
WidgetModel::addSubControl(const QString& control, QStringList widgets)
{
  if (m_subControls.contains(control)) {
    auto items = m_subControls.value(control);
    for (auto& name : widgets) {
      auto item = widgetItem(name);
      if (item && !item->hasSubControl(control)) {
        item->addSubcontrol(control);
        items.append(item);
      }
    }
    m_subControls.insert(control, items);
  } else {
    QList<WidgetItem*> items;
    for (auto& name : widgets) {
      auto item = widgetItem(name);
      item->addSubcontrol(control);
      items.append(item);
    }
    m_subControls.insert(control, items);
  }
}

void
WidgetModel::removeSubControl(const QString& control)
{
  if (m_subControls.contains(control)) {
    auto items = m_subControls.value(control);
    for (auto& item : items) {
      if (item && item->isExtraWidget()) {
        item->removeSubcontrol(control);
        items.removeOne(item);
      }
    }
    if (items.isEmpty()) {
      m_subControls.remove(control);
    } else {
      m_subControls.insert(control, items);
    }
  }
}

bool
WidgetModel::checkSubControlForWidget(const QString& widget,
                                      const QString& control)
{
  // first check direct widgets.
  auto items = m_subControls.value(control);
  if (!items.isEmpty()) {
    auto item = widgetItem(widget);
    if (items.contains(item)) {
      return true;
    }

    // then check for a subclassed widget.
    for (auto item : items) {
      while (item->parent() != nullptr) {
        if (item->name() == widget) {
          return true;
        }
      }
    }
  }

  return false;
}

bool
WidgetModel::isValidSubControlForWidget(const QString& widget,
                                        const QString& control)
{
  auto items = m_subControls.value(control);
  if (!items.isEmpty()) {
    for (auto item : items) {
      if (item->name() == widget) {
        return true;
      }
    }
  }

  return false;
}

bool
WidgetModel::isValidPropertyValueForProperty(const QString& propertyname,
                                             const QString& valuename)
{
  if (valuename.isEmpty()) {
    return false;
  }

  AttributeType propertyAttribute = m_attributes.value(propertyname);
  return (checkPropertyValue(propertyAttribute, valuename) != NoAttributeValue);
}

// bool
// WidgetModel::ifValidStylesheetValue(const QString& propertyname,
//                                    const QString& valuename,
//                                    StylesheetData* data)
//{
//  // TODO
//  //  if (valuename.isEmpty()) {
//  //    return false;
//  //  }

//  //  AttributeType stylesheetAttribute =
//  //    m_stylesheetAttributes.value(propertyname);
//  //  return (checkPropertyValue(stylesheetAttribute, valuename, data) !=
//  //          NoAttributeValue);
//  return false;
//}

bool
WidgetModel::containsSubControl(const QString& name)
{
  return m_subControls.contains(name.trimmed().toLower());
}

QStringList
WidgetModel::possibleWidgetsForSubControl(const QString& name)
{
  QStringList names;
  auto items = m_subControls.value(name);
  for (auto item : items) {
    names.append(recurseWidgetsForNames(item));
  }
  return names;
}

QStringList
WidgetModel::possibleSubControlsForWidget(const QString& widget)
{
  for (auto& item : m_widgets) {
    if (item->name() == widget) {
      return item->subControls();
    }
  }
  return QStringList();
}

void
WidgetModel::addPseudoState(const QString& state,
                            //                            QStringList widgets,
                            bool extraState)
{
  m_pseudoStates << state;
  m_pseudoStatesExtra << extraState;
}

void
WidgetModel::removePseudoState(const QString& state)
{
  auto i = m_pseudoStates.indexOf(state);
  if (m_pseudoStates.contains(state) && m_pseudoStatesExtra.at(i)) {
    m_pseudoStates.removeAt(i);
    m_pseudoStatesExtra.removeAt(i);
  }
}

bool
WidgetModel::containsPseudoState(const QString& name)
{
  return m_pseudoStates.contains(name);
}

// QStringList
// WidgetModel::pseudoStates()
//{
//  return m_pseudoStates;
//}

void
WidgetModel::addProperty(const QString& property, bool extraProperty)
{
  m_properties << property;
  m_propertiesExtra << extraProperty;
}

bool
WidgetModel::containsProperty(const QString& name)
{
  return m_properties.contains(name);
}

QStringList
WidgetModel::borderValues()
{
  QStringList values;
  values << m_borderStyle << m_borderImage << m_colors << m_paletteRoles
         << m_gradient;
  return values;
}

QMultiMap<int, QString>
WidgetModel::fuzzySearch(const QString& name, QStringList list)
{
  QMultiMap<int, QString> matches;
  char* pattern = new char[name.length() + 1];
  strcpy(pattern, name.toStdString().c_str());

  int score = 0;

  for (auto& valueStr : list) {
    char* value = new char[valueStr.size() + 1];
    strcpy(value, valueStr.toStdString().c_str());

    if (fts::fuzzy_match(pattern, value, score)) {
      matches.insert(score, valueStr);
    }

    delete[] value;
  }

  delete[] pattern;

  return matches;
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchWidgets(const QString& name)
{
  return fuzzySearch(name, m_widgets.keys());
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchProperty(const QString& name)
{
  return fuzzySearch(name, m_properties);
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchPropertyValue(const QString& name, const QString& value)
{
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
      QMap<int, QString> data;
      data.insert(fuzzySearch(value, m_colors));
      data.insert(fuzzySearch(value, m_paletteRoles));
      data.insert(fuzzySearch(value, m_gradient));
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

      //    case StylesheetEditGood:
      //      list << m_colors << "thin"
      //           << "extralight"
      //           << "light"
      //           << "normal"
      //           << "medium"
      //           << "demibold"
      //           << "bold"
      //           << "extrabold"
      //           << "black";
      //      return fuzzySearch(value, list);

      //    case StylesheetEditBad:
      //      list << m_colors << "thin"
      //           << "extralight"
      //           << "light"
      //           << "normal"
      //           << "medium"
      //           << "demibold"
      //           << "bold"
      //           << "extrabold"
      //           << "black"
      //           << "none"
      //           << "single"
      //           << "dash"
      //           << "dot"
      //           << "dashdot"
      //           << "dashdotdot"
      //           << "wave"
      //           << "spellcheck";
      //      return fuzzySearch(value, list);
  }
#pragma clang diagnostic pop
  return QMap<int, QString>();
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchPseudoStates(const QString& name)
{
  return fuzzySearch(name, m_pseudoStates);
}

QMultiMap<int, QString>
WidgetModel::fuzzySearchSubControl(const QString& name)
{
  return fuzzySearch(name, m_subControls.keys());
}

AttributeType
WidgetModel::propertyValueAttribute(const QString& value)
{
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

  } // TODO add property tests.
  /* else if (checkStylesheetEdit(value)) {
    return StylesheetEditGood;

  } else if (checkStylesheetEditBad(value)) {
    return StylesheetEditBad;
  }*/

  return NoAttributeValue;
}

QStringList
WidgetModel::eraseDuplicates(QStringList list)
{
  QStringList result;
  for (auto v : list) {
    if (!result.contains(v))
      result.append(v);
  }
  return result;
  // this doesn't seem to work with QList
  //  list->erase(std::unique(list->begin(), list->end()), list->end());
}

bool
WidgetModel::addCustomWidget(const QString& name, const QString& parent)
{
  if (hasWidget(parent)) {
    WidgetItem* parentItem = widgetItem(parent);
    auto item = new WidgetItem(name, parentItem);
    item->setExtraWidget(true);
    parentItem->addChild(item);
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetPseudoStates(const QString& widgetName,
                                         QStringList states)
{
  if (hasWidget(widgetName)) {
    auto item = widgetItem(widgetName);
    states = eraseDuplicates(states);
    //    for (auto state : states) {
    m_customPseudoStates.insert(widgetName, states);
    //    }
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetSubControls(const QString& widget,
                                        QStringList controls)
{
  if (m_customNames.contains(widget)) {
    controls = eraseDuplicates(controls);
    m_customSubControls.insert(widget, controls);
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetProperties(const QString& widget,
                                       QStringList properties)
{
  if (m_customNames.contains(widget)) {
    properties = eraseDuplicates(properties);
    m_customProperties.insert(widget, properties);
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetPropertyValue(const QString& widget,
                                          const QString& property,
                                          QString value)
{
  QMap<QString, QStringList> map;
  QStringList list;
  if (m_customValues.contains(widget)) {
    map = m_customValues.value(widget);
    if (map.contains(property)) {
      list = map.value(property);
    }
    list.append(value);
    list = eraseDuplicates(list);
    map.insert(property, list);
    m_customValues.insert(widget, map);
    return true;
  }
  return false;
}

bool
WidgetModel::addCustomWidgetPropertyValues(const QString& widget,
                                           const QString& property,
                                           QStringList values)
{
  if (m_customNames.contains(widget)) {
    QMap<QString, QStringList> map;
    QStringList list;
    if (m_customValues.contains(widget)) {
      map = m_customValues.value(widget);
      if (map.contains(property)) {
        list = map.value(property);
      }
      list.append(values);
    }
    list = eraseDuplicates(values);
    map.insert(property, list);
    m_customValues.insert(widget, map);
    return true;
  }
  return false;
}

QStringList
WidgetModel::recurseWidgetsForNames(WidgetItem* item)
{
  QStringList names;
  for (auto child : item->children()) {
    names.append(child->name());
    auto w = widgetItem(child->name());
    if (w && w->hasChildren()) {
      names.append(recurseWidgetsForNames(w));
    }
  }
  return names;
}

QString
Property::propertyName() const
{
  return m_propertyname;
}

void
Property::setPropertyName(const QString& propertyname)
{
  m_propertyname = propertyname;
}

AttributeTypes
Property::attributes() const
{
  return m_attributes;
}

void
Property::setAttributes(AttributeTypes attributes)
{
  m_attributes = attributes;
}
