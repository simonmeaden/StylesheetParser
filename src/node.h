/*
   Copyright 2020 Simon Meaden

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
#ifndef NODE_H
#define NODE_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QTextCursor>
#include <QWidget>
//#include <cstdarg>

#include "common.h"
#include "datastore.h"
#include "parserstate.h"

struct NodeData
{
  QTextCursor cursor;
  enum NodeType type;
  StylesheetEdit* parent;
  QString name;
};

class Node : public QObject
{
  Q_OBJECT
public:
  Node* previous;
  Node* next;

  explicit Node(const QString& name,
                QTextCursor cursor,
                StylesheetEdit* parent,
                enum NodeType type = NodeType);
  Node(const Node& other);
  ~Node();

  virtual int start() const;
  void setStart(int position);
  void moveStart(int count);
  virtual int end() const;
  QTextCursor cursor() const;
  virtual int length() const;

  QString name() const;
  void setName(const QString& value);

  virtual QPair<NodeSectionType, int> isIn(QPoint pos);

  enum NodeType type() const;
  QString toString() const;

protected:
  NodeData* d_ptr;
};

struct NameNodeData
{
  QString name;
};

class BadNode
{
public:
  explicit BadNode(ParserState::Errors errors);

  ParserState::Errors errors() const;
  void setError(const ParserState::Errors& errors);

private:
  ParserState::Errors m_errors;
};

class BadBlockNode
  : public Node
  , public BadNode
{
  Q_OBJECT
public:
  explicit BadBlockNode(const QString& name,
                        QTextCursor start,
                        ParserState::Errors errors,
                        StylesheetEdit* parent,
                        enum NodeType type = NodeType::BadNodeType);

  int end() const override;
};

class PropertyNode;
struct WidgetNodeData
{
  bool widgetValid = false;
  WidgetExtension extension = NoExtension;
  int markerOffset = 0;
  int extensionOffset = 0;
  QString extensionName;
  bool extensionValid = false;
  bool startBrace = false;
  int startBraceOffset = 0;
  bool endBrace = false;
  int endBraceOffset = 0;
  QList<PropertyNode*> properties;
};

class WidgetNode : public Node
{
  Q_OBJECT
public:
  explicit WidgetNode(const QString& name,
                      QTextCursor start,
                      StylesheetEdit* parent,
                      enum NodeType type = WidgetType);
  WidgetNode(const WidgetNode& other);
  ~WidgetNode();

  //  int end() const override;
  int length() const override;

  bool isWidgetValid() const;
  void setWidgetValid(bool widgetValid);

  QPair<NodeSectionType, int> isIn(QPoint pos) override;

  void setSubControl(int marker);
  void setPseudoState(int marker);
  int extensionOffset() const;
  void setExtensionOffset(int offset);
  int markerOffset() const;
  void setMarkerOffset(int offset);

  QString extensionName() const;
  void setExtensionName(const QString& name);
  bool isExtensionValid() const;
  bool isSubControl() const;
  bool isPseudoState() const;
  void setExtensionValid(bool valid);
  bool hasExtension() const;
  int extensionLength() const;

  bool hasStartBrace() const;
  void setStartBrace(int offset);
  int startBraceOffset() const;
  void removeStartBrace();
  bool hasEndBrace() const;
  void setEndBrace(int offset);
  int endBraceOffset() const;
  void removeEndBrace();
  bool offsetsMatch() const;

  void addProperty(PropertyNode* property);
  PropertyNode* property(int index) const;
  int propertyCount() const;
  //  QList<QTextCursor> propertyKeys();

private:
  WidgetNodeData* w_ptr;
};

struct PropertyNodeData
{
  QStringList values;
  QList<PropertyCheck> checks;
  QList<int> offsets;
  QList<DataStore::AttributeType> attributeTypes;
  bool propertyMarkerExists = false;
  int propertyMarkerOffset = 0;
  bool endMarkerExists = false;
  int endMarkerOffset = 0;
  bool validProperty = true;
};

class PropertyNode : public Node
{
  Q_OBJECT
public:
  explicit PropertyNode(const QString& name,
                        QTextCursor start,
                        StylesheetEdit* parent,
                        enum NodeType type = PropertyType);
  PropertyNode(const PropertyNode& other);
  ~PropertyNode();

  //! Returns the values as a list.
  QStringList values() const;
  //! Sets the values to the supplied string values.
  void setValues(const QStringList& values);
  //! Sets the value at index if index is valid.
  void setValue(int index, const QString& value);
  //! Adds a complete value/check/offset to the values.
  void addValue(const QString& value,
                PropertyCheck check,
                int offset,
                DataStore::AttributeType attType);

  //! Returns the checks as a list.
  QList<PropertyCheck> checks() const;
  //! Sets the checks to the supplied bool values.
  void setChecks(const QList<PropertyCheck>& checks);
  //! Sets the check at index if index is valid.
  void setCheck(int index, PropertyCheck check);

  //! Returns the offsets as a list.
  QList<int> offsets() const;
  //! Sets the offset at index if index is valid.
  void setOffset(int index, int offset);
  //! Sets the offsets to the supplied int offsets.
  void setOffsets(const QList<int>& offsets);
  void incrementOffsets(int increment = 1, int startIndex = 0);

  //! Returns the attribute types as a list.
  QList<DataStore::AttributeType> attributeTypes() const;
  //! Sets the attribute types as a list.
  void setAttributeTypes(const QList<DataStore::AttributeType>& attributeTypes);

  //  void correctValue(int index, const QString& value);
  //! Sets the check at index as bad.
  //!
  //! If index is not set then the default is to set the last value as bad.
  //!
  //! This will mean that the property at index will show up as a bad node, even
  //! if it is actually a correct value.
  bool setBadCheck(PropertyCheck check, int index = -1);

  //! Returns the number of values in the property.
  int count();
  // indicates whether the value at index is a valid value.
  PropertyCheck isValueValid(int index);
  int end() const override;
  int length() const override;

  bool isValidProperty() const;
  void setValidProperty(bool valid);

  bool hasPropertyMarker() const;
  void setPropertyMarkerExists(bool exists);
  int propertyMarkerOffset() const;
  void setPropertyMarkerOffset(int offset);
  void incrementPropertyMarker(int increment = 1);

  bool hasEndMarker();
  void setPropertyEndMarker(bool exists);
  int propertyEndMarkerOffset();
  void setPropertyEndMarkerOffset(int offset);
  void incrementEndMarkerOffset(int increment = 1);

  QPair<NodeSectionType, int> isIn(QPoint pos) override;
  PropertyStatus isProperty(int offset) const;

private:
  PropertyNodeData* p_ptr;
};

// class BadPropertyNode
//  : public PropertyNode
//  , public BadNode
//{
//  Q_OBJECT
// public:
//};

class NewlineNode : public Node
{
  Q_OBJECT
public:
  explicit NewlineNode(QTextCursor start,
                       StylesheetEdit* parent,
                       enum NodeType type = NewlineType);
};

class EndBraceNode;
class StartBraceNode : public Node
{
  Q_OBJECT
public:
  explicit StartBraceNode(QTextCursor start,
                          StylesheetEdit* parent,
                          enum NodeType type = StartBraceType);

  bool isBraceAtCursor() const;
  void setBraceAtCursor(bool isFlagBrace);

  EndBraceNode* endBrace() const;
  void setEndBrace(EndBraceNode* endBrace);
  bool hasEndBrace();

private:
  bool m_isBraceAtCursor;
  EndBraceNode* m_endBrace;
};

class EndBraceNode : public Node
{
  Q_OBJECT
public:
  explicit EndBraceNode(QTextCursor start,
                        StylesheetEdit* parent,
                        enum NodeType type = EndBraceType);

  bool isBraceAtCursor() const;
  void setBraceAtCursor(bool isFlagBrace);

  StartBraceNode* startBrace() const;
  void setStartNode(StartBraceNode* startNode);
  bool hasStartBrace();

private:
  bool m_isBraceAtCursor;
  StartBraceNode* m_startBrace;
};

struct CommentNodeData
{
  bool validComment = true;
  int offset = 0;
  bool endCommentExists = false;
  int endOffset = 0;
};

class CommentNode : public Node
{
  Q_OBJECT
public:
  explicit CommentNode(QTextCursor start,
                       StylesheetEdit* parent,
                       enum NodeType type = CommentType);
  CommentNode(const CommentNode& other);
  ~CommentNode();

  void append(QChar c);
  void append(QString text);
  int end() const override;
  int length() const override;
  int offset() const;
  void setOffset(int offset);

  bool hasEndComment() const;
  void setEndCommentExists(bool exists);
  int endCommentOffset() const;
  void setEndCommentOffset(int offset);

  QPair<NodeSectionType, int> isIn(QPoint pos) override;

private:
  CommentNodeData* c_ptr;
};

class BadStartCommentNode
  : public Node
  , public BadNode
{
  Q_OBJECT
public:
  explicit BadStartCommentNode(QTextCursor start,
                               ParserState::Errors errors,
                               StylesheetEdit* parent,
                               enum NodeType type = CommentStartMarkerType);

  int end() const override;
};

class BadEndCommentNode
  : public Node
  , public BadNode
{
  Q_OBJECT
public:
  explicit BadEndCommentNode(QTextCursor start,
                             ParserState::Errors errors,
                             StylesheetEdit* parent,
                             enum NodeType type = CommentEndMarkerType);

  int end() const override;
};

#endif // NODE_H
