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

#include "common.h"
#include "datastore.h"
#include "parserstate.h"

struct NodeData
{
  QTextCursor cursor;
  enum NodeType type;
  StylesheetEditor* editor;
  QString name;
  NodeChecks state = NodeCheck::BadNodeCheck;
};

class Node : public QObject
{
  Q_OBJECT
public:
  explicit Node(const QString& name,
                QTextCursor cursor,
                StylesheetEditor* editor,
                QObject* parent = nullptr,
                enum NodeType type = NodeType::NoType);
  Node(const Node& other);
  ~Node();

  virtual int end() const;
  QTextCursor cursor() const;
  void setCursor(QTextCursor podition);
  virtual int position() const;
  virtual int length() const;
  virtual int pointWidth() const;
  virtual int pointHeight() const;

  QString name() const;
  void setName(const QString& value);

  virtual NodeSection* isIn(QPoint pos);

  enum NodeType type() const;
  QString toString() const;

  NodeChecks checks() { return node_ptr->state; }
  void setCheck(NodeCheck check) { node_ptr->state.setFlag(check, true); }
  void clearCheck(NodeCheck check) { node_ptr->state.setFlag(check, false); }

protected:
  NodeData* node_ptr;
};

struct NameNodeData
{
  QString name;
};

// class BadNode
//{
// public:
//  explicit BadNode(ParserState::Errors errors);

//  ParserState::Errors errors() const;
//  void setError(const ParserState::Errors& errors);

// private:
//  ParserState::Errors m_errors;
//};

// class BadBlockNode
//  : public Node
//  , public BadNode
//{
//  Q_OBJECT
// public:
//  explicit BadBlockNode(const QString& name,
//                        QTextCursor start,
//                        ParserState::Errors errors,
//                        StylesheetEditor* editor,
//                        QObject* parent = nullptr,
//                        enum NodeType type = NodeType::BadNodeType);

//  int end() const override;
//};

class PropertyNode;

class Extension : public Node
{
public:
  Extension(QTextCursor markerCursor,
            QTextCursor nameCursor,
            const QString& name,
            StylesheetEditor* editor)
    : Node(name, nameCursor, editor)
    , m_markerCursor(markerCursor)
  {}

  QTextCursor markerCursor() const;
  void setMarkerCursor(const QTextCursor& markerCursor);

  QString marker() const;
  void setMarker(const QString& marker);
  int markerPosition() const { return m_markerCursor.position(); }

  virtual int length() const
  {
    return (name().length());
  }
  virtual bool isIn() {}

protected:
  QTextCursor m_markerCursor;
  QString m_marker;
};

class PseudoState : public Extension
{
public:
  PseudoState(QTextCursor markerCursor,
              QTextCursor nameCursor,
              const QString& name,
              StylesheetEditor* editor)
    : Extension(markerCursor, nameCursor, name, editor)
  {
    m_marker = ":";
  }
};

struct SubControl : public Extension
{
public:
  SubControl(QTextCursor markerCursor,
             QTextCursor nameCursor,
             const QString& name,
             StylesheetEditor* editor);

  QList<PseudoState*>* pseudoStates() const;
  void addPseudoState(PseudoState* pseudoStates);
  bool hasMarker() { return (checks().testFlag(SubControlMarkerCheck)); }
  int length() const;
  bool isFuzzy();

private:
  QList<PseudoState*>* m_pseudoStates = nullptr;
};

class WidgetNode : public Node
{
  Q_OBJECT

  struct WidgetNodeData
  {
    SubControl* subcontrol = nullptr;
    QList<PseudoState*> pseudoStates;
    QTextCursor startBracePosition;
    QTextCursor endBracePosition;
    QList<PropertyNode*> properties;
  };

public:
  explicit WidgetNode(const QString& name,
                      QTextCursor start,
                      StylesheetEditor* editor,
                      NodeCheck check = NodeCheck::BadNodeCheck,
                      QObject* parent = nullptr,
                      enum NodeType type = WidgetType);
  WidgetNode(const WidgetNode& other);
  ~WidgetNode();

  int length() const override;
  int pointWidth() const override { return Node::pointWidth(); }
  int pointHeight() const override { return Node::pointHeight(); }

  bool isValid() const;
  bool isNameValid() const;
  bool isNameFuzzy() const;
  //! \note Only changes widgetcheck or fuzzywidgetcheck.
  void setWidgetCheck(NodeCheck type);

  NodeSection* isIn(QPoint pos) override;

  SubControl* subControl() { return widget_ptr->subcontrol; }
  void setSubControl(SubControl* control) { widget_ptr->subcontrol = control; }
  QList<PseudoState*> pseudoStates() { return widget_ptr->pseudoStates; }
  void addPseudoState(PseudoState* state)
  {
    if (widget_ptr->pseudoStates.contains(state)) {
      widget_ptr->pseudoStates.replace(widget_ptr->pseudoStates.indexOf(state),
                                       state);
    } else {
      widget_ptr->pseudoStates.append(state);
    }
  }
  PseudoState* pseudoState(QTextCursor cursor)
  {
    for (auto state : widget_ptr->pseudoStates) {
      if (state->cursor() == cursor) {
        return state;
      }
    }
    return nullptr;
  }
  void setSubControlMarkerCursor(QTextCursor cursor);
  void setPseudoStateMarkerCursor(QTextCursor cursor);
  //  QTextCursor extensionCursor() const;
  //  int extensionPosition() const;
  //  void setExtensionCursor(QTextCursor cursor);
  //  QTextCursor markerCursor() const;
  //  int markerPosition() const;
  //  void setMarkerCursor(QTextCursor cursor);
  //  bool hasMarker();

  //  QString extensionName() const;
  //  void setExtensionName(const QString& name);
  bool isExtensionValid() const;
  bool isSubControlFuzzy() const;
  bool isExtensionBad() const;
  bool isSubControl() const;
  bool isPseudoState() const;
  //  void setExtensionType(enum NodeType type, NodeChecks check);
  bool hasSubControl() const;
  //  int subControlLength() const;

  bool hasStartBrace() const;
  void setStartBraceCursor(QTextCursor cursor);
  QTextCursor startBraceCursor() const;
  int startBracePosition() const;
  void removeStartBrace();
  bool hasEndBrace() const;
  void setEndBraceCursor(QTextCursor cursor);
  QTextCursor endBraceCursor() const;
  int endBracePosition() const;
  void removeEndBrace();
  bool hasMatchingBraces() const;

  bool doesMarkerMatch(NodeCheck type) const;
  bool doMarkersMatch() const;
  bool isExtensionMarkerCorrect();

  void addProperty(PropertyNode* property);
  PropertyNode* property(int index) const;
  int propertyCount() const;
  bool isFinalProperty(PropertyNode* property) const;
  bool arePropertiesValid() const;

private:
  WidgetNodeData* widget_ptr;
};

class PropertyNode : public Node
{
  Q_OBJECT
  Q_FLAGS(PropertyChecks)

  struct PropertyNodeData
  {
    QStringList values;
    QList<PropertyValueCheck> checks;
    QList<QTextCursor> cursors;
    QList<AttributeType> attributeTypes;

    NodeChecks state = NodeCheck::BadNodeCheck;
    QTextCursor propertyMarkerCursor;
    QTextCursor endMarkerCursor;
    WidgetNode* widget = nullptr;
    bool isFinalProperty = false;
  };

public:
  explicit PropertyNode(const QString& name,
                        QTextCursor start,
                        StylesheetEditor* editor,
                        enum NodeCheck check,
                        QObject* parent = nullptr,
                        enum NodeType type = PropertyType);
  PropertyNode(const PropertyNode& other);
  ~PropertyNode();

  void setWidget(WidgetNode* widget);
  bool hasWidget();

  //! Returns the values as a list.
  QStringList values() const;
  //! Sets the values to the supplied string values.
  void setValues(const QStringList& values);
  //! Sets the value at index if index is valid.
  void setValue(int index, const QString& value);
  //! Adds a complete value/check/offset to the values.
  void addValue(const QString& value,
                PropertyValueCheck check,
                QTextCursor offset,
                AttributeType attType);
  QString value(int index);

  //! Returns the checks as a list.
  QList<PropertyValueCheck> checks() const;
  //! Sets the checks to the supplied bool values.
  void setChecks(const QList<PropertyValueCheck>& checks);
  //! Sets the check at index if index is valid.
  void setCheck(int index, PropertyValueCheck check);
  PropertyValueCheck check(int index);

  //! Returns the positions as a list.
  QList<QTextCursor> valueCursors() const;
  //! Sets the position at index if index is valid.
  void setValueCursor(int index, QTextCursor position);
  //! Sets the positions to the supplied QTextCursor positions.
  void setValueCursors(const QList<QTextCursor>& positions);
  int valuePosition(int index);

  int pointWidth() const override { return Node::pointWidth(); }
  int pointHeight() const override { return Node::pointHeight(); }

  //! Returns the attribute types as a list.
  QList<AttributeType> attributeTypes() const;
  //! Sets the attribute types as a list.
  void setAttributeTypes(const QList<AttributeType>& attributeTypes);

  //! Sets the check at index as bad.
  //!
  //! If index is not set then the default is to set the last value as bad.
  //!
  //! This will mean that the property at index will show up as a bad node, even
  //! if it is actually a correct value.
  bool setBadCheck(PropertyValueCheck check, int index = -1);

  //! Returns the number of values in the property.
  int count();
  //! indicates whether the value at index is a valid value.
  bool isValueValid(int index);
  int end() const override;
  int length() const override;

  void setPropertyNameCheck(enum NodeCheck check);
  bool isValidPropertyName() const;
  bool isFuzzyName() const;
  //  void setValidPropertyName(bool valid);
  bool isValid(bool finalProperty = false);

  bool hasPropertyMarker() const;
  void setPropertyMarker(bool exists);
  QTextCursor propertyMarkerCursor() const;
  int propertyMarkerPosition() const;
  void setPropertyMarkerCursor(QTextCursor position);

  bool hasPropertyEndMarker() const;
  void setPropertyEndMarker(bool exists);
  QTextCursor propertyEndMarkerCursor() const;
  int propertyEndMarkerPosition() const;
  void setPropertyEndMarkerCursor(QTextCursor position);

  NodeSection* isIn(QPoint pos) override;

private:
  PropertyNodeData* property_ptr;
};

class NewlineNode : public WidgetNode
{
  Q_OBJECT
public:
  explicit NewlineNode(QTextCursor start,
                       StylesheetEditor* editor,
                       QObject* parent = nullptr,
                       enum NodeType type = NewlineType);
};

// class EndBraceNode;
// class StartBraceNode : public WidgetNode
//{
//  Q_OBJECT
// public:
//  explicit StartBraceNode(QTextCursor start,
//                          StylesheetEditor* editor,
//                          QObject* parent = nullptr,
//                          enum NodeType type = StartBraceType);

//  bool isBraceAtCursor() const;
//  void setBraceAtCursor(bool isFlagBrace);

//  EndBraceNode* endBrace() const;
//  void setEndBrace(EndBraceNode* endBrace);
//  bool hasEndBrace();

// private:
//  bool m_isBraceAtCursor;
//  EndBraceNode* m_endBrace;
//};

// class EndBraceNode : public WidgetNode
//{
//  Q_OBJECT
// public:
//  explicit EndBraceNode(QTextCursor start,
//                        StylesheetEditor* editor,
//                        QObject* parent = nullptr,
//                        enum NodeType type = EndBraceType);

//  bool isBraceAtCursor() const;
//  void setBraceAtCursor(bool isFlagBrace);

//  StartBraceNode* startBrace() const;
//  void setStartNode(StartBraceNode* startNode);
//  bool hasStartBrace();

// private:
//  bool m_isBraceAtCursor;
//  StartBraceNode* m_startBrace;
//};

struct CommentNodeData
{
  bool validComment = true;
  QTextCursor cursor;
  bool endCommentExists = false;
  QTextCursor endCursor;
};

class CommentNode : public WidgetNode
{
  Q_OBJECT
public:
  explicit CommentNode(QTextCursor start,
                       StylesheetEditor* editor,
                       QObject* parent = nullptr,
                       enum NodeType type = CommentType);
  CommentNode(const CommentNode& other);
  ~CommentNode();

  void append(QChar c);
  void append(QString text);
  int end() const override;
  int length() const override;
  QTextCursor textCursor() const;
  void setTextCursor(QTextCursor cursor);
  int textPosition() const;

  bool hasEndComment() const;
  void setEndCommentExists(bool exists);
  QTextCursor endCommentCursor() const;
  void setEndCommentCursor(QTextCursor cursor);

  NodeSection* isIn(QPoint pos) override;

private:
  CommentNodeData* comment_ptr;
};

// class BadStartCommentNode
//  : public WidgetNode
//  , public BadNode
//{
//  Q_OBJECT
// public:
//  explicit BadStartCommentNode(QTextCursor start,
//                               ParserState::Errors errors,
//                               StylesheetEditor* editor,
//                               QObject* parent = nullptr,
//                               enum NodeType type = CommentStartMarkerType);

//  int end() const override;
//};

// class BadEndCommentNode
//  : public WidgetNode
//  , public BadNode
//{
//  Q_OBJECT
// public:
//  explicit BadEndCommentNode(QTextCursor start,
//                             ParserState::Errors errors,
//                             StylesheetEditor* editor,
//                             QObject* parent = nullptr,
//                             enum NodeType type = CommentEndMarkerType);

//  int end() const override;
//};

#endif // NODE_H
