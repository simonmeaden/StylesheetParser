/*
  Copyright 2020 Simon Meaden

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
        */
#ifndef COMMON_H
#define COMMON_H

#include <QColor>
#include <QFont>
#include <QList>
#include <QString>
#include <QTextCharFormat>
#include <QTextCursor>

class Node;
//#include "node.h"

template<typename T>
class asKeyValueRange
{
public:
  asKeyValueRange(T& data)
    : m_data{ data }
  {}

  auto begin() { return m_data.keyValueBegin(); }

  auto end() { return m_data.keyValueEnd(); }

private:
  T& m_data;
};

struct ThemeData
{
  ThemeData(const QString& fore,
            const QString& back,
            const QString& italic,
            const QString& bold,
            const QString& underlineColor,
            const QString& underlineStyle)
    : m_foreground(fore)
    , m_background(back)
    , m_italic(ThemeData::toBool(italic))
    , m_bold(ThemeData::toBool(bold))
    , m_underlineColor(underlineColor)
    , m_underlineStyle(ThemeData::stringToStyle(underlineStyle))
  {}

  QString m_foreground;
  QString m_background;
  bool m_italic;
  bool m_bold;
  QString m_underlineColor;
  QTextCharFormat::UnderlineStyle m_underlineStyle;

  static QTextCharFormat::UnderlineStyle stringToStyle(const QString& style)
  {
    if (style == "NoUnderline")
      return QTextCharFormat::NoUnderline;
    else if (style == "SingleUnderline")
      return QTextCharFormat::SingleUnderline;
    else if (style == "DotLine")
      return QTextCharFormat::DotLine;
    else if (style == "DashDotLine")
      return QTextCharFormat::DashDotLine;
    else if (style == "DashDotDotLine")
      return QTextCharFormat::DashDotDotLine;
    else if (style == "DashUnderline")
      return QTextCharFormat::DashUnderline;
    else if (style == "WaveUnderline")
      return QTextCharFormat::WaveUnderline;
    else if (style == "SpellCheckUnderline")
      return QTextCharFormat::SpellCheckUnderline;
    return QTextCharFormat::NoUnderline;
  }
  static bool toBool(const QString& value)
  {
    if (value.toLower() == "true")
      return true;
    return false;
  }
};

class Theme
{
public:
  QMap<QString, ThemeData> data() const { return m_data; }
  void addData(QString name, ThemeData& data) { m_data.insert(name, data); }

private:
  QMap<QString, ThemeData> m_data;
};

class StylesheetData
{
public:
  QString name;
  QList<QColor> colors;
  QList<QFont::Weight> weights;
  QList<QTextCharFormat::UnderlineStyle> underline;
};

enum SectionType
{
  None,
  Name,
  PropertyName,
  FuzzyPropertyName,
  PropertyValue,      // only valid for propety nodes.
  FuzzyPropertyValue, // only valid for propety nodes.
  PropertyMarker,
  PropertyEndMarker,
  WidgetName,
  FuzzyWidgetName,
  PseudoStateName,
  PseudoStateMarker,
  SubControlPseudoStateName,
  SubControlPseudoStateMarker,
  WidgetSubControlPseudoStateName,
  WidgetSubControlPseudoStateMarker,
  WidgetPseudoState,
  FuzzyWidgetPseudoState,
  SubControlName,
  SubControlMarker,
  WidgetSubControlName,
  FuzzyWidgetSubControl,
  BadWidgetSubControl,
  WidgetPropertyName,
  WidgetPropertyMarker,
  WidgetPropertyValue,
  WidgetPropertyEndMarker,
  WidgetPseudoStateMarker,
  WidgetSubControlMarker,
  WidgetStartBrace,
  WidgetEndBrace,
  Comment,
};

struct PartialType
{
  enum Type
  {
    Name,
    Marker,
    Value,
    EndMarker,
  };

  int start = -1;
  int end = -1;
  int offset = -1;
  Type type;
};

class NodeSection : public QObject
{
public:
  NodeSection(QObject* parent, SectionType t = None, int p = -1, int i = -1)
    : QObject(parent)
    , type(t)
    , position(p)
    , node(nullptr)
    , propertyIndex(i)
  {}
  bool isPropertyType()
  {
    switch (type) {
      case PropertyName:
      case FuzzyPropertyName:
      case PropertyMarker:
      case PropertyEndMarker:
      case PropertyValue:
      case FuzzyPropertyValue:
        return true;
      default:
        return false;
    }
  }
  bool isWidgetType()
  {
    switch (type) {
      case WidgetName:
      case FuzzyWidgetName:
      case WidgetPseudoStateMarker:
      case FuzzyWidgetPseudoState:
      case WidgetSubControlMarker:
      case FuzzyWidgetSubControl:
      case WidgetPseudoState:
      case WidgetSubControlName:
      case WidgetPropertyName:
      case WidgetPropertyValue:
      case WidgetPropertyMarker:
      case WidgetPropertyEndMarker:
      case WidgetStartBrace:
      case WidgetEndBrace:
        return true;
      default:
        return false;
    }
  }
  void clear()
  {
    type = None;
    position = -1;
    propertyIndex - 1;
    node = nullptr;
  }
  bool isCommentType()
  {
    switch (type) {
      case Comment:
        return true;
      default:
        return false;
    }
  }

  friend bool operator==(NodeSection& left, NodeSection& right);
  friend bool operator!=(NodeSection& left, NodeSection& right);
  SectionType type;
  int position;
  Node* node;
  int propertyIndex;
};

bool
operator==(NodeSection& left, NodeSection& right);
bool
operator!=(NodeSection& left, NodeSection& right);

enum NodeType
{
  NoType,
  //  NodeType,
  WidgetType,
  WidgetsType,
  //  FuzzyWidgetType,
  PseudoStateType,
  //  FuzzyPseudoStateType,
  PseudoStateMarkerType,
  SubControlType,
  //  FuzzySubControlType,
  SubControlMarkerType,
  IdSelectorType,
  IdSelectorMarkerType,
  ColonType, //! Either a PropertyMarkerType or a PseudoStateMarkerType
  StartBraceType,
  EndBraceType,
  NewlineType,
  PropertyType,
  //  FuzzyPropertyType,
  PropertyMarkerType,
  PropertyValueType,
  //  FuzzyPropertyValueType,
  BadNodeType,
  CommentType,
  CommentStartMarkerType,
  CommentEndMarkerType,
};

QDebug
operator<<(QDebug debug, const NodeType& type);

struct NodeStateData
{
  bool stateValid;
};

enum AttributeType
{
  NoAttributeValue,
  Alignment,
  Attachment,
  Background,
  Bool,    //!< This is true/false value
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
  //  UnderlineColor,
  //  UnderlineStyle,
  //  Custom,
};
Q_DECLARE_FLAGS(AttributeTypes, AttributeType);
Q_DECLARE_OPERATORS_FOR_FLAGS(AttributeTypes)

enum NodeState
{
  BadNodeState = 0,
  PropertyMarkerState = 0x1,
  PropertyEndMarkerState = 0x2,
  ValidNameState = 0x4,
  ValidPropertyValueState = 0x8,
  FuzzyPropertyState = 0x10,
  FuzzyPropertyNameState = 0x20,
  FuzzyPropertyValueState = 0x40,
  BadPropertyValueState = 0x80,
  GoodPropertyState = 0xF,
  //
  //  WidgetExtensionCheck = 0x10,
  //
  SubControlState = 0x100,
  FuzzySubControlState = 0x200,
  SubControlMarkerState = 0x400,
  BadSubControlForWidgetState = 0x800,
  SubControlSeperatorState = 0x1000,
  IdSelectorState = 0x2000,
  ValidPropertyNameState = 0x4000,
  //
  PseudostateState = 0x10000,
  FuzzyPseudostateState = 0x20000,
  PseudostateMarkerState = 0x40000,
  IdSelectorMarkerState = 0x80000,
  //
  StartBraceState = 0x100000,
  EndBraceState = 0x200000,
  //
  WidgetState = 0x1000000,
  FuzzyWidgetState = 0x2000000,
  //
  CommentState = 0x10000000,
  NewLineState = 0x20000000,

};
Q_DECLARE_FLAGS(NodeStates, NodeState);
Q_DECLARE_OPERATORS_FOR_FLAGS(NodeStates)

enum NodeIsIn
{
  InNode,
  BeforeNode,
  AfterNode,
};

struct CursorData
{
  QTextCursor cursor;
  Node* node = nullptr;
  Node* prevNode = nullptr;
};

struct MenuData
{
  Node* node;
  //  QPoint pos;
  SectionType type;
  QString oldName = QString();
};
Q_DECLARE_METATYPE(MenuData);

enum PropertyValueState
{
  //  IrrelevantValue, // Not relevant for this check.
  GoodName, //! Good value for this property name
  BadName,
  FuzzyName,
  GoodValueName,
  BadValueName,
  FuzzyValueName,
  GoodValue,
  BadValue,
  BadValueCount,
  BadNumericalValue,
  //  BadNumericalValue_Max,
  BadNumericalValue_31,
  BadNumericalValue_100,
  BadNumericalValue_255,
  BadNumericalValue_359,
  BadButtonLayoutValue,
  BadLengthUnit,
  BadFontUnit,
  BadColorValue,
  BadUrlValue,
  RepeatValueName,
  FuzzyColorValue,
  //  CloseParentheses,
};

class PropertyStatus
{
  //  static const QStringList names;
  struct Internal
  {
    QString name;
    int offset;
    PropertyValueState state;
    QRect rect;
  };

public:
  PropertyStatus(PropertyValueState state = PropertyValueState::GoodValue,
                 const QString& name = QString(),
                 int offset = -1)
    : m_name(name)
    , m_state(state)
    , m_offset(offset)
  {}

  ~PropertyStatus() { qDeleteAll(m_internals); }

  int length() const { return name().length(); }
  PropertyStatus* lastStatus()
  {
    auto status = m_next;
    while (status) {
      if (status->m_next)
        status = status->m_next;
      else
        return status;
    }
    return nullptr;
  }
  int lastOffset()
  {
    auto next = lastStatus();
    if (next)
      return next->m_offset;
    else
      return m_offset;
  }
  int lastEnd()
  {
    auto next = lastStatus();
    if (next)
      return next->m_offset + next->length();
    else
      return m_offset + length();
  }

  int sectionCount() { return m_internals.size(); }
  QString name() const { return m_name; }
  QString sectionName(int index)
  {
    if (index >= 0 && index < m_internals.length())
      return m_internals.at(index)->name;
    return QString();
  }
  void setName(const QString name) { m_name = name; }
  void setSectionName(int index, const QString name)
  {
    if (index > 0 && index < m_internals.length())
      m_internals.at(index)->name = name;
  }

  PropertyValueState state() const { return m_state; }
  PropertyValueState sectionState(int index) const
  {
    return m_internals.at(index)->state;
  }
  bool isGoodValue() { return (m_state == GoodValue); }
  void setState(const PropertyValueState& state) { m_state = state; }
  void setSectionState(int index, const PropertyValueState& state)
  {
    if (index >= 0 && index < m_internals.length())
      m_internals.at(index)->state = state;
  }

  int offset() const { return m_offset; }
  int sectionOffset(int index) const
  {
    if (index >= 0 && index < m_internals.length())
      return m_internals.at(index)->offset;
    return -1;
  }
  void setOffset(int offset) { m_offset = offset; }
  void setSectionOffset(int index, int offset)
  {
    if (index >= 0 && index < m_internals.length())
      m_internals.at(index)->offset = offset;
  }
  int sectionLength(int index) { return m_internals.at(index)->name.length(); }

  QRect rect() const { return m_rect; }
  bool isInRect(QPoint pos)
  {
    auto x = pos.x();
    auto y = pos.y();
    if (x >= m_rect.left() && x < m_rect.right() && y >= m_rect.top() &&
        y < m_rect.bottom()) {
      return true;
    }
    return false;
  }
  QRect sectionRect(int index) const
  {
    if (index >= 0 && index < m_internals.length())
      return m_internals.at(index)->rect;
    return QRect();
  }
  bool isInSectionRect(int index, QPoint pos)
  {
    if (index > 0 && index < m_internals.length()) {
      auto x = pos.x();
      auto y = pos.y();
      auto rect = m_internals.at(index)->rect;
      if (x >= rect.left() && x < rect.right() && y >= rect.top() &&
          y < rect.bottom()) {
        return true;
      }
    }
    return false;
  }
  void setRect(const QRect& rect) { m_rect = rect; }
  void setSectionRect(int index, const QRect& rect)
  {
    if (index > 0 && index < m_internals.length())
      m_internals.at(0)->rect = rect;
  }

  PropertyStatus* next() { return m_next; }
  void setNext(PropertyStatus* status) { m_next = status; }

  void addSectionValue(const PropertyValueState& state,
                       const QString& name,
                       int offset,
                       const QRect& rect)
  {
    auto internal = new Internal();
    internal->offset = offset;
    internal->name = name;
    internal->state = state;
    internal->rect = rect;
    m_internals.append(internal);
  }

  PropertyValueState sectionsState()
  {
    // if ANY internal state is not good return bad.
    for (auto internal : m_internals) {
      if (!(internal->state == GoodValue || internal->state == GoodName)) {
        return BadValue;
      }
    }
    return GoodValue;
  }

private:
  PropertyValueState m_state;
  QString m_name;
  int m_offset;
  PropertyStatus* m_next = nullptr;
  QRect m_rect;
  QList<Internal*> m_internals;
};

QDebug
operator<<(QDebug debug, const PropertyStatus& status);

#endif // COMMON_H
