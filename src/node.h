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

class Node : public QObject
{
  Q_OBJECT
public:
  Node(QTextCursor cursor,
       StylesheetEditor* editor,
       QObject* parent,
       enum NodeType type = NoType);

  Node(const Node& other);

  virtual QTextCursor cursor() const;
  virtual void setCursor(QTextCursor podition);
  virtual int position() const;
  virtual int length() const;
  virtual int end() const;
  virtual bool isIn(int pos) = 0;
  virtual bool isIn(QPoint pos) = 0;
  virtual NodeSection* sectionIfIn(QPoint pos) = 0;

  enum NodeType type() const;

protected:
  QTextCursor m_cursor;
  enum NodeType m_type = NoType;
  StylesheetEditor* m_editor = nullptr;
  NodeStates m_state = BadNodeState;
};

QDebug
operator<<(QDebug debug, const Node& node);

class NamedNode : public Node
{
  Q_OBJECT
public:
  NamedNode(const QString& name,
            QTextCursor cursor,
            StylesheetEditor* editor,
            QObject* parent,
            enum NodeType type = NoType);
  NamedNode(const NamedNode& other);
  ~NamedNode();

  int length() const;
  int end() const;
  bool isIn(int pos);
  bool isIn(QPoint pos);
  NodeSection* sectionIfIn(QPoint pos);

  QString name() const;
  void setName(const QString& value);

  QString toString() const;

  NodeStates state() const;
  void setStateFlag(NodeState flag);
  void clearStateFlag(NodeState check);

protected:
  QString m_name;

  virtual int pointWidth(const QString& text) const;
  virtual int pointHeight() const;
};

QDebug
operator<<(QDebug debug, const NamedNode& node);

class MarkerBase : public NamedNode
{
  Q_OBJECT
public:
  MarkerBase(QTextCursor markerCursor,
             QTextCursor nameCursor,
             const QString& name,
             StylesheetEditor* editor,
             QObject* parent,
             enum NodeType type = NoType);

  QTextCursor cursor() const;
  QTextCursor nameCursor() const;
  void setCursor(QTextCursor markerCursor);
  void setNameCursor(QTextCursor cursor);

  QString marker() const;
  int position() const;
  int namePosition() const { return NamedNode::position(); }

  virtual bool isFuzzy() const;

  int length() const override;

protected:
  QTextCursor m_markerCursor;
  QString m_marker;
};

class PseudoState : public MarkerBase
{
  Q_OBJECT
public:
  PseudoState(QTextCursor markerCursor,
              QTextCursor nameCursor,
              const QString& name,
              StylesheetEditor* editor,
              QObject* parent,
              enum NodeType type = PseudoStateType);

  bool hasMarker();
  bool isFuzzy() const override;
  bool isValid() const;

  bool isIn(QPoint pos) override;
  NodeSection* sectionIfIn(QPoint pos) override;

protected:
  int pointWidth(const QString& text) const override;
};

class ControlBase : public MarkerBase
{
  Q_OBJECT
public:
  ControlBase(QTextCursor markerCursor,
              QTextCursor nameCursor,
              const QString& name,
              StylesheetEditor* editor,
              QObject* parent,
              enum NodeType type = SubControlType);

  QList<PseudoState*>* pseudoStates() const;
  bool hasPseudoStates();
  PseudoState* pseudoState(QTextCursor cursor);
  void addPseudoState(PseudoState* pseudoStates);
  bool hasMarker();
  int length() const;
  bool isFuzzy() const override;
  virtual bool isValid() const;
  virtual NodeSection* sectionIfIn(QPoint pos);

protected:
  int pointWidth(const QString& text) const override;

private:
  QList<PseudoState*>* m_pseudoStates = nullptr;
};

class SubControl : public ControlBase
{
  Q_OBJECT
public:
  SubControl(QTextCursor markerCursor,
             QTextCursor nameCursor,
             const QString& name,
             StylesheetEditor* editor,
             QObject* parent,
             enum NodeType type = IdSelectorType);
};

class IDSelector : public ControlBase
{
  Q_OBJECT
public:
  IDSelector(QTextCursor markerCursor,
             QTextCursor nameCursor,
             const QString& name,
             StylesheetEditor* editor,
             QObject* parent,
             enum NodeType type = IdSelectorType);

  bool isValid() const override;
};

class WidgetNodes;

class PropertyNode : public NamedNode
{
  Q_FLAGS(PropertyChecks)
  Q_OBJECT

public:
  explicit PropertyNode(const QString& name,
                        QTextCursor start,
                        StylesheetEditor* editor,
                        enum NodeState check,
                        QObject* parent,
                        enum NodeType type = PropertyType);
  PropertyNode(const PropertyNode& other);
  ~PropertyNode();

  void setWidgetNodes(WidgetNodes* widget);
  bool hasWidgetNodes();

  //! Returns the values as a list.
  QStringList values() const;
  //! Sets the values to the supplied string values.
  void setValues(const QStringList& values);
  //! Sets the value at index if index is valid.
  void setValue(int index, const QString& value);
  //! Adds a complete value/check/offset to the values.
  void addValue(const QString& value,
                NodeState check,
                QTextCursor cursor,
                PropertyStatus* status);
  QString value(int index);

  //! Returns the checks as a list.
  QList<NodeState> state() const;
  //! Sets the checks to the supplied bool values.
  void setChecks(const QList<NodeState>& checks);
  //! Sets the check at index if index is valid.
  void setStateFlag(int index, NodeState check);
  NodeState check(int index);

  //! Returns the positions as a list.
  QList<QTextCursor> valueCursors() const;
  //! Sets the position at index if index is valid.
  void setValueCursor(int index, QTextCursor position);
  //! Sets the positions to the supplied QTextCursor positions.
  void setValueCursors(const QList<QTextCursor>& positions);
  int valuePosition(int index);

  //! Returns the attribute types as a list.
  //  QList<PropertyValueState> valueStatus() const;
  PropertyStatus* valueStatus(int index) const;
  //  //! Sets the attribute types as a list.
  //  void setAttributeTypes(const QList<AttributeType>& attributeTypes);

  //! Sets the check at index as bad.
  //!
  //! If index is not set then the default is to set the last value as bad.
  //!
  //! This will mean that the property at index will show up as a bad node, even
  //! if it is actually a correct value.
  //  bool setBadCheck(PropertyValueCheck check, int index = -1);

  //! Returns the number of values in the property.
  int count();
  //! indicates whether the value at index is a valid value.
  bool isValueValid(int index);
  int end() const override;
  int length() const override;

  void setPropertyNameCheck(enum NodeState check);
  bool isValidPropertyName() const;
  bool isFuzzyName() const;
  bool isFinalProperty();
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

  NodeSection* sectionIfIn(QPoint pos) override;
  QList<PartialType> sectionIfIn(int start, int end);

protected:
  QStringList m_values;
  QList<NodeState> m_checks;
  QList<QTextCursor> m_cursors;
  QList<PropertyStatus*> m_valueStatus;

  NodeStates m_propertyState = NodeState::BadNodeState;
  QTextCursor m_propertyMarkerCursor;
  QTextCursor m_endMarkerCursor;
  WidgetNodes* m_widgetnodes = nullptr;
  bool m_isFinalProperty = false;
};

class NewlineNode : public NamedNode
{
  Q_OBJECT
public:
  explicit NewlineNode(QTextCursor start,
                       StylesheetEditor* editor,
                       QObject* parent = nullptr,
                       enum NodeType type = NewlineType);
};

class CommentNode : public Node
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

  NodeSection* sectionIfIn(QPoint pos) override;
  bool isIn(int pos) override { return false; }
  bool isIn(QPoint pos) override { return false; }

private:
  bool m_validComment = true;
  QTextCursor m_textCursor;
  bool m_endCommentExists = false;
  QTextCursor m_endCursor;
  QString m_text;
};

class WidgetNode : public NamedNode
{
  Q_OBJECT
public:
  explicit WidgetNode(const QString& name,
                      QTextCursor start,
                      StylesheetEditor* editor,
                      NodeState check,
                      QObject* parent,
                      enum NodeType type = WidgetType);
  WidgetNode(const WidgetNode& other);
  ~WidgetNode();

  int length() const override;

  bool isValid() const;
  bool isNameValid() const;
  bool isNameFuzzy() const;
  //! \note Only changes widgetcheck or fuzzywidgetcheck.
  void setWidgetCheck(NodeState type);

  bool isIn(QPoint pos) override;
  NodeSection* sectionIfIn(QPoint pos) override;

  QList<SubControl*>* subControls();
  SubControl* subControl(QPoint pos) const;
  SubControl* subControl(QTextCursor cursor) const;
  bool hasSubControls() const;
  void addSubControl(SubControl* control);

  void addPseudoState(PseudoState* state);
  bool hasPseudoStates();

  PseudoState* pseudoState(QPoint pos) const;
  PseudoState* pseudoState(QTextCursor cursor) const;
  void setSubControlMarkerCursor(QTextCursor cursor);
  void setPseudoStateMarkerCursor(QTextCursor cursor);

  IDSelector* idSelector();
  void setIdSelector(IDSelector* selector);
  bool hasIdSelector();

  bool isSubControlValid(QPoint pos) const;
  bool isSubControlFuzzy(QTextCursor cursor) const;
  bool isSubControlFuzzy(QPoint pos) const;
  bool isSubControlBad(QPoint pos) const;
  bool isSubControl() const;
  bool isPseudoState() const;
  bool hasSubControl() const;

  bool doesMarkerMatch(NodeState type) const;
  bool doMarkersMatch() const;
  bool isExtensionMarkerCorrect();

protected:
  QList<SubControl*>* m_subcontrols = nullptr;
  QList<PseudoState*>* m_pseudoStates = nullptr;
  IDSelector* m_idSelector = nullptr;
};

class WidgetNodes : public Node
{
  Q_OBJECT
public:
  explicit WidgetNodes(QTextCursor cursor,
                       StylesheetEditor* editor,
                       QObject* parent)
    : Node(cursor, editor, parent, WidgetsType)
  {}

  void addWidget(WidgetNode* widget) { m_widgets.append(widget); }
  QList<WidgetNode*> widgets() { return m_widgets; }

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

  void addProperty(PropertyNode* property);
  PropertyNode* property(int index) const;
  int propertyCount() const;
  bool arePropertiesValid() const;
  bool isFinalProperty(PropertyNode* property) const;

  void addSeperatorCursor(QTextCursor cursor);
  QList<QTextCursor> seperators();

  int length() const;
  int end() const;
  bool isIn(int pos);
  bool isIn(QPoint pos);
  NodeSection* sectionIfIn(QPoint pos);

protected:
  QList<WidgetNode*> m_widgets;
  QTextCursor m_startBracePosition;
  QTextCursor m_endBracePosition;
  QList<PropertyNode*> m_properties;
  QList<QTextCursor> m_widgetSeperators;

  bool isInStartBrace(QPoint pos);
  bool isInEndBrace(QPoint pos);
};

#endif // NODE_H
