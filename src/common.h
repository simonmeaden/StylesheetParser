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

class Node;

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
  PropertyValue, // only valid for propety nodes.
  PropertyMarker,
  PropertyEndMarker,
  WidgetName,
  WidgetPseudoState,
  FuzzyWidgetPseudoState,
  WidgetSubControl,
  FuzzyWidgetSubControl,
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

struct NodeSection
{
  NodeSection(SectionType t = None, int p = -1, int i = -1)
    : type(t)
    , position(p)
    , node(nullptr)
    , propertyIndex(i)
  {}
  bool isPropertyType()
  {
    switch (type) {
      case PropertyName:
      case PropertyMarker:
      case PropertyEndMarker:
      case PropertyValue:
        return true;
      default:
        return false;
    }
  }
  bool isWidgetType()
  {
    switch (type) {
      case WidgetName:
      case WidgetPseudoStateMarker:
      case WidgetSubControlMarker:
      case WidgetPseudoState:
      case WidgetSubControl:
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
  //  FuzzyWidgetType,
  PseudoStateType,
  //  FuzzyPseudoStateType,
  PseudoStateMarkerType,
  SubControlType,
  //  FuzzySubControlType,
  SubControlMarkerType,
  ColonType, //! Either a PropertyMarkerType or a PseudoStateMarkerType
//  StartBraceType,
//  EndBraceType,
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
  // below here are specific to StylesheetEdit
  StylesheetEditGood,
  StylesheetEditBad,
};
enum NodeCheck
{
  BadNodeCheck = 0,
  PropertyMarkerCheck = 0x1,
  PropertyEndMarkerCheck = 0x2,
  ValidNameCheck = 0x4,
  FuzzyPropertyCheck = 0x8,
  InvalidNameCheck = 0x10,
  GoodPropertyCheck = 0x7,
  //
  //  WidgetExtensionCheck = 0x10,
  //
  SubControlCheck = 0x100,
  FuzzySubControlCheck = 0x200,
  SubControlMarkerCheck = 0x400,
  //
  PseudoStateCheck = 0x1000,
  FuzzyPseudoStateCheck = 0x2000,
  PseudoStateMarkerCheck = 0x4000,
  //
  StartBraceCheck = 0x10000,
  EndBraceCheck = 0x20000,
  //
  WidgetCheck = 0x100000,
  FuzzyWidgetCheck = 0x200000,
  //
  CommentCheck = 0x1000000,
  //
  NewLineCheck = 0x10000000,

};
Q_DECLARE_FLAGS(NodeChecks, NodeCheck);
Q_DECLARE_OPERATORS_FOR_FLAGS(NodeChecks)

enum PropertyValueCheck
{
  NoCheck,
  GoodValue,
  ValidPropertyType,
  BadValue,
};

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

struct PropertyStatus
{
  PropertyStatus(SectionType status = SectionType::None,
                 const QString& name = QString(),
                 int offset = -1)
    : m_status(status)
    , m_offset(offset)
    , m_name(name)
  {}

  SectionType m_status;
  int m_offset;
  QString m_name;

public:
  SectionType status() const;
  int offset() const;
  QString name() const;
};

#endif // COMMON_H
