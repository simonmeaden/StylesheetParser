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
#include "node.h"

#include <QtDebug>

#include "stylesheetedit_p.h"
#include "stylesheetparser/stylesheetedit.h"

Node::Node(const QString& name,
           QTextCursor cursor,
           StylesheetEditor* editor,
           QObject* parent,
           enum NodeType type)
  : QObject(parent)
{
  node_ptr = new NodeData;
  node_ptr->cursor = cursor;
  node_ptr->type = type;
  node_ptr->editor = editor;
  node_ptr->name = name;
}

Node::Node(const Node& other)
{
  node_ptr = new NodeData(*other.node_ptr);
}

Node::~Node()
{
  delete node_ptr;
}

int
Node::end() const
{
  return position() + length();
}

int
Node::pointWidth(const QString& text) const
{
  auto fm = node_ptr->editor->fontMetrics();
  return fm.horizontalAdvance(text);
}

int
Node::pointHeight() const
{
  auto fm = node_ptr->editor->fontMetrics();
  return fm.height();
}

QTextCursor
Node::cursor() const
{
  return node_ptr->cursor;
}

void
Node::setCursor(QTextCursor cursor)
{
  node_ptr->cursor = cursor;
}

int
Node::position() const
{
  return node_ptr->cursor.anchor();
}

enum NodeType
Node::type() const
{
  return node_ptr->type;
}

QString
Node::toString() const
{
  switch (node_ptr->type) {
    case WidgetType:
      return node_ptr->name;
      //    case StartBraceType:
      //      return "{";
      //    case EndBraceType:
      //      return "}";
    case NewlineType:
      return "\n";
    case PropertyType:
      return node_ptr->name;
    case CommentType:
      return node_ptr->name;
    case BadNodeType:
      return "";
  }

  return QString();
}

NodeChecks
Node::checks() const
{
  return node_ptr->state;
}

void
Node::setCheck(NodeCheck check)
{
  node_ptr->state.setFlag(check, true);
}

void
Node::clearCheck(NodeCheck check)
{
  node_ptr->state.setFlag(check, false);
}

QString
Node::name() const
{
  return node_ptr->name;
}

void
Node::setName(const QString& value)
{
  node_ptr->name = value;
}

int
Node::length() const
{
  return node_ptr->name.length();
}

bool
Node::isIn(int pos)
{
  auto p = position();
  auto e = end();
  if (pos >= p && pos < e) {
    return true;
  }
  return false;
}

bool
Node::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = node_ptr->editor->cursorRect(QTextCursor(node_ptr->cursor));
  auto left = rect.x();
  auto width = pointWidth(node_ptr->name);
  auto height = pointHeight();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;

  if (x >= left && x <= right && y >= top && y <= bottom) {
    return true;
  }
  return false;
}

NodeSection*
Node::sectionIfIn(QPoint pos)
{
  if (isIn(pos)) {
    return new NodeSection(this, SectionType::Name);
  }

  return new NodeSection(this);
}

WidgetNode::WidgetNode(const QString& name,
                       QTextCursor start,
                       StylesheetEditor* editor,
                       enum NodeCheck check,
                       QObject* parent,
                       enum NodeType type)
  : Node(name, start, editor, parent, type)
{
  widget_ptr = new WidgetNodeData();
  setWidgetCheck(check);
}

WidgetNode::WidgetNode(const WidgetNode& other)
  : Node(other.name(),
         other.cursor(),
         other.node_ptr->editor,
         other.parent(),
         other.type())
{
  widget_ptr = new WidgetNodeData(*other.widget_ptr);
}

WidgetNode::~WidgetNode()
{
  delete widget_ptr;
}

int
WidgetNode::length() const
{
  if (hasEndBrace()) {
    return endBraceCursor().anchor() - cursor().anchor();
  } else if (widget_ptr->properties.size() > 0) {
    return widget_ptr->properties.last()->length();
  } else if (hasStartBrace()) {
    return startBraceCursor().anchor() - cursor().anchor();
  } else if (hasSubControl()) {
    return widget_ptr->subcontrols->last()->cursor().anchor() -
           cursor().anchor() + widget_ptr->subcontrols->length();
  } else {
    return name().length();
  }
}

bool
WidgetNode::isValid() const
{
  return (node_ptr->state.testFlag(NodeCheck::WidgetCheck) &&
          (!hasSubControl() || (hasSubControl() && doMarkersMatch())) &&
          arePropertiesValid() && hasStartBrace() && hasEndBrace());
}

bool
WidgetNode::isNameValid() const
{
  return (node_ptr->state.testFlag(NodeCheck::WidgetCheck));
}

bool
WidgetNode::isNameFuzzy() const
{
  return (node_ptr->state.testFlag(NodeCheck::FuzzyWidgetCheck));
}

void
WidgetNode::setWidgetCheck(NodeCheck check)
{
  switch (check) {
    case NodeCheck::WidgetCheck: {
      node_ptr->state.setFlag(NodeCheck::WidgetCheck, true);
      node_ptr->state.setFlag(NodeCheck::FuzzyWidgetCheck, false);
      break;
    }
    case NodeCheck::FuzzyWidgetCheck: {
      node_ptr->state.setFlag(NodeCheck::FuzzyWidgetCheck, true);
      node_ptr->state.setFlag(NodeCheck::WidgetCheck, false);
      break;
    }
    case NodeCheck::SubControlCheck: {
      node_ptr->state.setFlag(NodeCheck::SubControlCheck, true);
      node_ptr->state.setFlag(NodeCheck::FuzzySubControlCheck, false);
      node_ptr->state.setFlag(NodeCheck::PseudoStateCheck, false);
      node_ptr->state.setFlag(NodeCheck::FuzzyPseudoStateCheck, false);
      break;
    }
    case NodeCheck::FuzzySubControlCheck: {
      node_ptr->state.setFlag(NodeCheck::SubControlCheck, false);
      node_ptr->state.setFlag(NodeCheck::FuzzySubControlCheck, true);
      node_ptr->state.setFlag(NodeCheck::PseudoStateCheck, false);
      node_ptr->state.setFlag(NodeCheck::FuzzyPseudoStateCheck, false);
      break;
    }
    case NodeCheck::PseudoStateCheck: {
      node_ptr->state.setFlag(NodeCheck::SubControlCheck, false);
      node_ptr->state.setFlag(NodeCheck::FuzzySubControlCheck, false);
      node_ptr->state.setFlag(NodeCheck::PseudoStateCheck, true);
      node_ptr->state.setFlag(NodeCheck::FuzzyPseudoStateCheck, false);
      break;
    }
    case NodeCheck::FuzzyPseudoStateCheck: {
      node_ptr->state.setFlag(NodeCheck::SubControlCheck, false);
      node_ptr->state.setFlag(NodeCheck::FuzzySubControlCheck, false);
      node_ptr->state.setFlag(NodeCheck::PseudoStateCheck, false);
      node_ptr->state.setFlag(NodeCheck::FuzzyPseudoStateCheck, true);
      break;
    }
  }
}

bool
WidgetNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = node_ptr->editor->cursorRect(QTextCursor(node_ptr->cursor));
  auto left = rect.x();
  auto width = pointWidth(node_ptr->name);
  auto height = pointHeight();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;

  if (x >= left && x <= right && y >= top && y <= bottom) {
    return true;
  }
  return false;
}

NodeSection*
WidgetNode::sectionIfIn(QPoint pos)
{

  // this just checks the property name.
  auto section = Node::sectionIfIn(pos);
  if (section->type == SectionType::Name) {
    section->type = SectionType::WidgetName;
    return section;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = node_ptr->editor->fontMetrics();
  int top, bottom;
  QTextCursor cursor;

  if (hasSubControl()) {
    for (auto subcontrol : *(widget_ptr->subcontrols)) {
      section = subcontrol->sectionIfIn(pos);
      if (section->type == SubControlName) {
        section->type = WidgetSubControlName;
        return section;
      } else if (section->type == SubControlMarker) {
        section->type = WidgetSubControlMarker;
        return section;
      } else if (section->type == SubControlPseudoStateName) {
        section->type = WidgetSubControlPseudoStateName;
        return section;
      } else if (section->type == SubControlPseudoStateMarker) {
        section->type = WidgetSubControlPseudoStateMarker;
        return section;
      }
    }
  }

  if (hasStartBrace()) {
    cursor = startBraceCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance("{");
    if (x >= left && x <= right && y >= top && y < bottom) {
      section->type = SectionType::WidgetStartBrace;
      return section;
    }
  }

  for (int i = 0; i < propertyCount(); i++) {
    PropertyNode* p = property(i);
    section = p->sectionIfIn(pos);
    if (section->isPropertyType()) {
      switch (section->type) {
        case SectionType::PropertyName:
          section->type = SectionType::WidgetPropertyName;
          break;
        case SectionType::PropertyMarker:
          section->type = SectionType::WidgetPropertyMarker;
          break;
        case SectionType::PropertyValue:
          section->type = SectionType::WidgetPropertyValue;
          break;
        case SectionType::PropertyEndMarker:
          section->type = SectionType::WidgetPropertyEndMarker;
          break;
      }
      section->propertyIndex = i;
      return section;
    }
  }

  if (hasEndBrace()) {
    cursor = endBraceCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance("}");
    if (x >= left && x <= right && y >= top && y < bottom) {
      section->type = SectionType::WidgetEndBrace;
      return section;
    }
  }

  return section;
}

QList<SubControl*>*
WidgetNode::subControls()
{
  return widget_ptr->subcontrols;
}

SubControl*
WidgetNode::subControl(QPoint pos) const
{
  for (auto subcontrol : *(widget_ptr->subcontrols)) {
    if (subcontrol->isIn(pos)) {
      return subcontrol;
    }
  }
  return nullptr;
}

SubControl*
WidgetNode::subControl(QTextCursor cursor) const
{
  if (hasSubControls()) {
    for (auto control : *(widget_ptr->subcontrols)) {
      if (control->cursor() == cursor || control->cursor() == cursor) {
        return control;
      }
    }
  }
  return nullptr;
}

bool
WidgetNode::hasSubControls() const
{
  if (widget_ptr->subcontrols && !widget_ptr->subcontrols->isEmpty()) {
    return true;
  }
  return false;
}

void
WidgetNode::addSubControl(SubControl* control)
{
  if (!widget_ptr->subcontrols)
    widget_ptr->subcontrols = new QList<SubControl*>();

  if (!widget_ptr->subcontrols->contains(control)) {
    widget_ptr->subcontrols->append(control);
  } else {
    widget_ptr->subcontrols->replace(widget_ptr->subcontrols->indexOf(control),
                                     control);
  }
}

void
WidgetNode::addPseudoState(PseudoState* state)
{
  if (!widget_ptr->pseudoStates)
    widget_ptr->pseudoStates = new QList<PseudoState*>();

  if (!widget_ptr->pseudoStates->contains(state)) {
    widget_ptr->pseudoStates->append(state);
  } else {
    widget_ptr->pseudoStates->replace(widget_ptr->pseudoStates->indexOf(state),
                                      state);
  }
}

bool
WidgetNode::hasPseudoStates()
{
  if (widget_ptr->pseudoStates && !widget_ptr->pseudoStates->isEmpty()) {
    return true;
  }
  return false;
}

PseudoState*
WidgetNode::pseudoState(QPoint pos) const
{
  for (auto state : *(widget_ptr->pseudoStates)) {
    if (state->isIn(pos)) {
      return state;
    }
  }
  return nullptr;
}

PseudoState*
WidgetNode::pseudoState(QTextCursor cursor) const
{
  if (widget_ptr->pseudoStates) {
    for (auto state : *(widget_ptr->pseudoStates)) {
      if (state->cursor() == cursor || state->cursor() == cursor) {
        return state;
      }
    }
  }
  return nullptr;
}

// PropertyStatus
// WidgetNode::sectionAtOffset(int pos) const
//{
//  if (pos >= node_ptr->cursor.anchor() &&
//      pos < node_ptr->cursor.anchor() + node_ptr->name.length()) {
//    return PropertyStatus(SectionType::WidgetPropertyName, node_ptr->name);

//  } else {
//    //    for (int i = 0; i < p_ptr->values.size(); i++) {
//    //      auto value = p_ptr->values.at(i);
//    //      auto offset = p_ptr->cursors.at(i).anchor();

//    //      if (pos >= offset && pos < offset + value.length()) {
//    //        return PropertyStatus(SectionType::WidgetPropertyValue, value,
//    //        offset);
//    //      }
//    //    }
//  }

//  return PropertyStatus();
//}

void
WidgetNode::setSubControlMarkerCursor(QTextCursor cursor)
{
  subControl(cursor)->setCursor(cursor);
  node_ptr->state.setFlag(NodeCheck::SubControlMarkerCheck, true);
}

void
WidgetNode::setPseudoStateMarkerCursor(QTextCursor cursor)
{
  pseudoState(cursor)->setCursor(cursor);
  node_ptr->state.setFlag(NodeCheck::PseudoStateMarkerCheck, true);
}

bool
WidgetNode::doesMarkerMatch(enum NodeCheck type) const
{
  if (type == NodeCheck::PseudoStateCheck ||
      type == NodeCheck::FuzzyPseudoStateCheck) {
    if (node_ptr->state.testFlag(NodeCheck::PseudoStateMarkerCheck)) {
      return true;
    }
  } else if (type == NodeCheck::SubControlCheck ||
             type == NodeCheck::FuzzySubControlCheck) {
    if (node_ptr->state.testFlag(NodeCheck::SubControlMarkerCheck)) {
      return true;
    }
  }
  return false;
}

bool
WidgetNode::doMarkersMatch() const
{
  return (doesMarkerMatch(NodeCheck::PseudoStateCheck) ||
          doesMarkerMatch(NodeCheck::SubControlCheck));
}

// bool WidgetNode::isSubControlValid(QTextCursor cursor) const
//{
//  if (hasSubControl()) {
//    auto subcontrol = subControl(cursor);
//    auto checks = subcontrol->checks();
//    if (checks.testFlag(NodeCheck::SubControlCheck) &&
//        !checks.testFlag(NodeCheck::BadSubControlForWidgetCheck)) {
//      return true;
//    }
//  }
//  return false;
//}

bool
WidgetNode::isSubControlValid(QPoint pos) const
{
  if (hasSubControl()) {
    auto subcontrol = subControl(pos);
    auto checks = subcontrol->checks();
    if (checks.testFlag(NodeCheck::SubControlCheck) &&
        !checks.testFlag(NodeCheck::BadSubControlForWidgetCheck)) {
      return true;
    }
  }
  return false;
}

bool
WidgetNode::isSubControlFuzzy(QTextCursor cursor) const
{
  auto subcontrol = subControl(cursor);
  if (subcontrol) {
    return (node_ptr->state.testFlag(NodeCheck::FuzzySubControlCheck));
  }
  return false;
}

bool
WidgetNode::isSubControlFuzzy(QPoint pos) const
{
  auto subcontrol = subControl(pos);
  if (subcontrol) {

    return (node_ptr->state.testFlag(NodeCheck::FuzzySubControlCheck));
  }
  return false;
}

bool
WidgetNode::isSubControlBad(QPoint pos) const
{
  auto subcontrol = subControl(pos);
  if (subcontrol) {
    return (subControl(pos)->checks().testFlag(
      NodeCheck::BadSubControlForWidgetCheck));
  }
  return true;
}

bool
WidgetNode::isSubControl() const
{
  return (node_ptr->state.testFlag(NodeCheck::SubControlCheck) ||
          node_ptr->state.testFlag(NodeCheck::FuzzySubControlCheck));
}

bool
WidgetNode::isPseudoState() const
{
  return (node_ptr->state.testFlag(NodeCheck::PseudoStateCheck) ||
          node_ptr->state.testFlag(NodeCheck::FuzzyPseudoStateCheck));
}

// void
// WidgetNode::setExtensionType(enum NodeType type, NodeChecks checks)
//{
//  switch (type) {
////    case NodeType::SubControlType:
////      if (checks.testFlag(NodeCheck::SubControlCheck)) {
////        node_ptr->state.setFlag(NodeCheck::SubControlCheck, true);
////        node_ptr->state.setFlag(NodeCheck::FuzzySubControlCheck, false);
////      } else if (checks.testFlag(NodeCheck::FuzzySubControlCheck)) {
////        node_ptr->state.setFlag(NodeCheck::SubControlCheck, false);
////        node_ptr->state.setFlag(NodeCheck::FuzzySubControlCheck, true);
////      }
////      if (checks.testFlag(NodeCheck::BadSubControlForWidgetCheck)) {
////        node_ptr->state.setFlag(NodeCheck::BadSubControlForWidgetCheck);
////      }
////      node_ptr->state.setFlag(NodeCheck::PseudoStateCheck, false);
////      node_ptr->state.setFlag(NodeCheck::FuzzyPseudoStateCheck, false);
////      break;
//    case NodeType::PseudoStateType:
//      if (checks.testFlag(NodeCheck::PseudoStateCheck)) {
//        node_ptr->state.setFlag(NodeCheck::PseudoStateCheck, true);
//        node_ptr->state.setFlag(NodeCheck::FuzzyPseudoStateCheck, false);
//      } else if (checks.testFlag(NodeCheck::FuzzyPseudoStateCheck)) {
//        node_ptr->state.setFlag(NodeCheck::PseudoStateCheck, false);
//        node_ptr->state.setFlag(NodeCheck::FuzzyPseudoStateCheck, true);
//      }
//      node_ptr->state.setFlag(NodeCheck::SubControlCheck, false);
//      node_ptr->state.setFlag(NodeCheck::FuzzySubControlCheck, false);
//      break;
//  }
//}

bool
WidgetNode::hasSubControl() const
{
  return (widget_ptr->subcontrols);
  //  return (node_ptr->state.testFlag(NodeCheck::SubControlCheck) ||
  ////          node_ptr->state.testFlag(NodeCheck::PseudoStateCheck) ||
  //          node_ptr->state.testFlag(NodeCheck::FuzzySubControlCheck)/* ||
  //          node_ptr->state.testFlag(NodeCheck::FuzzyPseudoStateCheck)*/);
}

// int
// WidgetNode::subControlLength() const
//{
//  return widget_ptr->subcontrol->length();
//}

bool
WidgetNode::isExtensionMarkerCorrect()
{
  return ((node_ptr->state.testFlag(PseudoStateCheck) &&
           node_ptr->state.testFlag(PseudoStateMarkerCheck)) ||
          (node_ptr->state.testFlag(SubControlCheck) &&
           node_ptr->state.testFlag(SubControlMarkerCheck)));
}

bool
WidgetNode::hasStartBrace() const
{
  return (node_ptr->state.testFlag(NodeCheck::StartBraceCheck));
}

void
WidgetNode::setStartBraceCursor(QTextCursor cursor)
{
  node_ptr->state.setFlag(NodeCheck::StartBraceCheck, true);
  widget_ptr->startBracePosition = cursor;
}

QTextCursor
WidgetNode::startBraceCursor() const
{
  return widget_ptr->startBracePosition;
}

int
WidgetNode::startBracePosition() const
{
  return widget_ptr->startBracePosition.anchor();
}

void
WidgetNode::removeStartBrace()
{
  (node_ptr->state.setFlag(NodeCheck::StartBraceCheck, false));
}

bool
WidgetNode::hasEndBrace() const
{
  return (node_ptr->state.testFlag(NodeCheck::EndBraceCheck));
}

void
WidgetNode::setEndBraceCursor(QTextCursor cursor)
{
  node_ptr->state.setFlag(NodeCheck::EndBraceCheck, true);
  widget_ptr->endBracePosition = cursor;
}

QTextCursor
WidgetNode::endBraceCursor() const
{
  return widget_ptr->endBracePosition;
}

int
WidgetNode::endBracePosition() const
{
  return widget_ptr->endBracePosition.anchor();
}

void
WidgetNode::removeEndBrace()
{
  node_ptr->state.setFlag(NodeCheck::EndBraceCheck, false);
}

bool
WidgetNode::hasMatchingBraces() const
{
  return (node_ptr->state.testFlag(NodeCheck::StartBraceCheck) &&
          node_ptr->state.testFlag(NodeCheck::EndBraceCheck));
}

void
WidgetNode::addProperty(PropertyNode* property)
{
  widget_ptr->properties.append(property);
}

PropertyNode*
WidgetNode::property(int index) const
{
  return widget_ptr->properties.at(index);
}

int
WidgetNode::propertyCount() const
{
  return widget_ptr->properties.size();
}

bool
WidgetNode::isFinalProperty(PropertyNode* property) const
{
  auto index = widget_ptr->properties.indexOf(property);
  return (index == widget_ptr->properties.size() - 1);
}

bool
WidgetNode::arePropertiesValid() const
{
  for (auto property : widget_ptr->properties) {
    bool final = (property == widget_ptr->properties.last());
    if (!property->isValid(final)) {
      return false;
    }
  }
  return true;
}

PropertyNode::PropertyNode(const QString& name,
                           QTextCursor start,
                           StylesheetEditor* editor,
                           NodeCheck check,
                           QObject* parent,
                           enum NodeType type)
  : Node(name, start, editor, parent, type)
{
  property_ptr = new PropertyNodeData;
  setPropertyNameCheck(check);
}

PropertyNode::PropertyNode(const PropertyNode& other)
  : Node(other.name(),
         other.cursor(),
         other.node_ptr->editor,
         other.parent(),
         other.type())
{
  property_ptr = new PropertyNodeData(*other.property_ptr);
}

PropertyNode::~PropertyNode()
{
  delete property_ptr;
}

void
PropertyNode::setWidget(WidgetNode* widget)
{
  property_ptr->widget = widget;
}

bool
PropertyNode::hasWidget()
{
  return (property_ptr->widget);
}

QStringList
PropertyNode::values() const
{
  return property_ptr->values;
}

QList<NodeCheck>
PropertyNode::checks() const
{
  return property_ptr->checks;
}

QList<QTextCursor>
PropertyNode::valueCursors() const
{
  return property_ptr->cursors;
}

void
PropertyNode::setValueCursor(int index, QTextCursor cursor)
{
  if (index >= 0 && index < property_ptr->cursors.length()) {
    property_ptr->cursors.replace(index, cursor);
  }
}

QList<AttributeType>
PropertyNode::attributeTypes() const
{
  return property_ptr->attributeTypes;
}

void
PropertyNode::setValues(const QStringList& values)
{
  property_ptr->values = values;
}

void
PropertyNode::setValue(int index, const QString& value)
{
  if (index >= 0 && index < property_ptr->values.length()) {
    property_ptr->values.replace(index, value);
  }
}

void
PropertyNode::setChecks(const QList<NodeCheck>& checks)
{
  property_ptr->checks = checks;
}

void
PropertyNode::setCheck(int index, NodeCheck check)
{
  if (index >= 0 && index < property_ptr->checks.length()) {
    property_ptr->checks.replace(index, check);
  }
}

NodeCheck
PropertyNode::check(int index)
{
  if (index >= 0 && index < property_ptr->values.size())
    return property_ptr->checks.at(index);
  return BadNodeCheck;
}

void
PropertyNode::setValueCursors(const QList<QTextCursor>& positions)
{
  property_ptr->cursors = positions;
}

int
PropertyNode::valuePosition(int index)
{
  if (index >= 0 && index < property_ptr->cursors.size())
    return property_ptr->cursors.at(index).anchor();
  return -1;
}

void
PropertyNode::addValue(const QString& value,
                       NodeCheck check,
                       QTextCursor position,
                       AttributeType attType = NoAttributeValue)
{
  property_ptr->values.append(value);
  property_ptr->checks.append(check);
  property_ptr->cursors.append(position);
  property_ptr->attributeTypes.append(attType);
}

QString
PropertyNode::value(int index)
{
  if (index >= 0 && index < property_ptr->values.size())
    return property_ptr->values.at(index);
  return QString();
}

// bool
// PropertyNode::setBadCheck(PropertyValueCheck check, int index)
//{
//  if (index == -1 && !property_ptr->checks.isEmpty()) {
//    property_ptr->checks[property_ptr->checks.length() - 1] = check;
//    return true;

//  } else if (index >= 0 && index < property_ptr->checks.length()) {
//    property_ptr->checks[index] = check;
//    return true;
//  }

//  return false;
//}

int
PropertyNode::count()
{
  return property_ptr->cursors.size();
}

bool
PropertyNode::isValueValid(int index)
{
  if (index >= 0 && index < count()) {
    return property_ptr->checks.at(index) != BadNodeCheck;
  }

  // default to bad.
  return false;
}

int
PropertyNode::end() const
{
  return position() + length();
}

int
PropertyNode::length() const
{
  if (property_ptr->cursors.isEmpty()) {
    if (!hasPropertyMarker()) {
      return node_ptr->name.length();
    } else {
      return propertyMarkerCursor().anchor() + 1;
    }
  } else {
    return property_ptr->cursors.last().anchor() - node_ptr->cursor.anchor() +
           property_ptr->values.last().length();
  }

  return 0;
}

void
PropertyNode::setPropertyNameCheck(NodeCheck check)
{
  if (check == ValidNameCheck) {
    property_ptr->state.setFlag(check, true);
    property_ptr->state.setFlag(FuzzyPropertyCheck, false);
  } else if (check == FuzzyPropertyCheck) {
    property_ptr->state.setFlag(ValidNameCheck, false);
    property_ptr->state.setFlag(check, true);
  } else if (check == BadNodeCheck) {
    property_ptr->state.setFlag(ValidNameCheck, false);
    property_ptr->state.setFlag(FuzzyPropertyCheck, false);
  }
}

bool
PropertyNode::hasPropertyMarker() const
{
  return property_ptr->state.testFlag(NodeCheck::PropertyMarkerCheck);
}

void
PropertyNode::setPropertyMarker(bool exists)
{
  property_ptr->state.setFlag(NodeCheck::PropertyMarkerCheck, exists);
}

bool
PropertyNode::isValidPropertyName() const
{
  return (property_ptr->state.testFlag(NodeCheck::ValidNameCheck));
}

bool
PropertyNode::isFuzzyName() const
{
  return (property_ptr->state.testFlag(NodeCheck::FuzzyPropertyCheck));
}

bool
PropertyNode::isValid(bool finalProperty)
{
  if (property_ptr->widget) {
    if (property_ptr->widget->isFinalProperty(this)) {
      return isValidPropertyName() && hasPropertyMarker();
    }
  }

  if (!hasPropertyEndMarker() && finalProperty) {
    return isValidPropertyName() && hasPropertyMarker();
  }

  return property_ptr->state.testFlag(ValidNameCheck);
}

QTextCursor
PropertyNode::propertyMarkerCursor() const
{
  return property_ptr->propertyMarkerCursor;
}

int
PropertyNode::propertyMarkerPosition() const
{
  return property_ptr->propertyMarkerCursor.anchor();
}

void
PropertyNode::setPropertyMarkerCursor(QTextCursor position)
{
  property_ptr->propertyMarkerCursor = position;
}

bool
PropertyNode::hasPropertyEndMarker() const
{
  return property_ptr->state.testFlag(NodeCheck::PropertyEndMarkerCheck);
}

void
PropertyNode::setPropertyEndMarker(bool exists)
{
  property_ptr->state.setFlag(NodeCheck::PropertyEndMarkerCheck, exists);
}

QTextCursor
PropertyNode::propertyEndMarkerCursor() const
{
  return property_ptr->endMarkerCursor;
}

int
PropertyNode::propertyEndMarkerPosition() const
{
  return property_ptr->endMarkerCursor.anchor();
}

void
PropertyNode::setPropertyEndMarkerCursor(QTextCursor position)
{
  property_ptr->endMarkerCursor = position;
}

NodeSection*
PropertyNode::sectionIfIn(QPoint pos)
{
  // this just checks the property name.
  auto isin = Node::sectionIfIn(pos);
  if (isin->type == SectionType::Name) {
    isin->type = SectionType::PropertyName;
    return isin;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = node_ptr->editor->fontMetrics();
  int top, bottom;
  QTextCursor cursor;

  // check marker;
  if (hasPropertyMarker()) {
    cursor = propertyMarkerCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance(":");
    if (x >= left && x <= right && y >= top && y < bottom) {
      isin->type = SectionType::PropertyMarker;
      return isin;
    }
  }

  // then value(s)
  QString value;
  for (int i = 0; i < valueCursors().count(); i++) {
    value = values().at(i);
    cursor = valueCursors().at(i);
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance(value);

    if (x >= left && x <= right && y >= top && y < bottom) {
      isin->type = SectionType::PropertyValue;
      isin->position = i;
      break;
    }
  }

  if (hasPropertyEndMarker()) {
    cursor = propertyEndMarkerCursor();
    rect = node_ptr->editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance(":");
    if (x >= left && x <= right && y >= top && y < bottom) {
      isin->type = SectionType::PropertyEndMarker;
      return isin;
    }
  }

  return isin;
}

void
PropertyNode::setAttributeTypes(const QList<AttributeType>& attributeTypes)
{
  property_ptr->attributeTypes = attributeTypes;
}

CommentNode::CommentNode(QTextCursor start,
                         StylesheetEditor* editor,
                         QObject* parent,
                         enum NodeType type)
  : WidgetNode(QString(), start, editor, NodeCheck::CommentCheck, parent, type)
// TODO set actual comment check value
{
  comment_ptr = new CommentNodeData;
}

CommentNode::CommentNode(const CommentNode& other)
  : WidgetNode(other.name(),
               other.cursor(),
               other.node_ptr->editor,
               NodeCheck::CommentCheck, // TODO set actual comment check value
               other.parent(),
               other.type())
{
  comment_ptr = new CommentNodeData(*other.comment_ptr);
}

CommentNode::~CommentNode()
{
  delete comment_ptr;
}

void
CommentNode::append(QChar c)
{
  node_ptr->name.append(c);
}

void
CommentNode::append(QString text)
{
  node_ptr->name.append(text);
}

int
CommentNode::end() const
{
  return node_ptr->cursor.anchor() + node_ptr->name.length();
}

int
CommentNode::length() const
{
  if (hasEndComment()) {
    return endCommentCursor().anchor() - position() + 2;
  } else if (node_ptr->name.length() > 0) {
    return comment_ptr->cursor.anchor() + node_ptr->name.length() - position();
  } else {
    return 2; // length of start comment
  }
}

QTextCursor
CommentNode::textCursor() const
{
  return comment_ptr->cursor;
}

void
CommentNode::setTextCursor(QTextCursor cursor)
{
  comment_ptr->cursor = cursor;
}

int
CommentNode::textPosition() const
{
  return comment_ptr->cursor.anchor();
}

bool
CommentNode::hasEndComment() const
{
  return comment_ptr->endCommentExists;
}

void
CommentNode::setEndCommentExists(bool exists)
{
  comment_ptr->endCommentExists = exists;
}

QTextCursor
CommentNode::endCommentCursor() const
{
  return comment_ptr->endCursor;
}

void
CommentNode::setEndCommentCursor(QTextCursor cursor)
{
  comment_ptr->endCursor = cursor;
}

NodeSection*
CommentNode::sectionIfIn(QPoint pos)
{
  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = node_ptr->editor->fontMetrics();
  int top, bottom;
  NodeSection* isin = new NodeSection(this);

  // check marker;
  QTextCursor s(node_ptr->editor->document());
  s.setPosition(position());
  rect = node_ptr->editor->cursorRect(s);
  top = rect.y();
  bottom = top + rect.height();
  left = rect.x();
  s.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, length());
  rect = node_ptr->editor->cursorRect(s);
  right = rect.x();
  if (x >= left && x <= right && y >= top && y < bottom) {
    isin->type = SectionType::Comment;
    return isin;
  }

  return isin;
}

NewlineNode::NewlineNode(QTextCursor start,
                         StylesheetEditor* editor,
                         QObject* parent,
                         enum NodeType type)
  : WidgetNode("\n", start, editor, NodeCheck::NewLineCheck, parent, type)
{}

MarkerBase::MarkerBase(QTextCursor markerCursor,
                       QTextCursor nameCursor,
                       const QString& name,
                       StylesheetEditor* editor)
  : Node(name, nameCursor, editor)
  , m_markerCursor(markerCursor)
{}

QTextCursor
MarkerBase::cursor() const
{
  return m_markerCursor;
}

QTextCursor
MarkerBase::nameCursor() const
{
  return Node::cursor();
}

void
MarkerBase::setCursor(QTextCursor markerCursor)
{
  m_markerCursor = markerCursor;
}

void
MarkerBase::setNameCursor(QTextCursor cursor)
{
  Node::setCursor(cursor);
}

QString
MarkerBase::marker() const
{
  return m_marker;
}

int
MarkerBase::position() const
{
  return m_markerCursor.position();
}

int
MarkerBase::length() const
{
  return (m_markerCursor.position() - Node::position() + name().length());
}

SubControl::SubControl(QTextCursor markerCursor,
                       QTextCursor nameCursor,
                       const QString& name,
                       StylesheetEditor* editor)
  : MarkerBase(markerCursor, nameCursor, name, editor)
{
  m_marker = "::";
  if (!markerCursor.isNull()) {
    setCheck(NodeCheck::SubControlMarkerCheck);
  }
}

QList<PseudoState*>*
SubControl::pseudoStates() const
{
  return m_pseudoStates;
}

bool
SubControl::hasPseudoStates()
{
  if (m_pseudoStates && !m_pseudoStates->isEmpty()) {
    return true;
  }
  return false;
}

PseudoState*
SubControl::pseudoState(QTextCursor cursor)
{
  if (m_pseudoStates) {
    for (auto state : *(m_pseudoStates)) {
      if (state->cursor() == cursor) {
        return state;
      } else if (state->cursor() == cursor) {
        return state;
      }
    }
  }
  return nullptr;
}

void
SubControl::addPseudoState(PseudoState* state)
{
  if (!m_pseudoStates)
    m_pseudoStates = new QList<PseudoState*>();

  if (!m_pseudoStates->contains(state)) {
    m_pseudoStates->append(state);
  } else {
    m_pseudoStates->replace(m_pseudoStates->indexOf(state), state);
  }
}

int
SubControl::length() const
{
  if (m_pseudoStates && !m_pseudoStates->isEmpty()) {
    auto state = m_pseudoStates->last();
    return (state->position() + state->length() - position());
  } else {
    return MarkerBase::length();
  }
}

bool
SubControl::isFuzzy() const
{
  return (checks().testFlag(FuzzySubControlCheck));
}

bool
SubControl::isValid() const
{
  if (node_ptr->state.testFlag(FuzzySubControlCheck) ||
      !node_ptr->state.testFlag(SubControlMarkerCheck) ||
      node_ptr->state.testFlag(BadSubControlForWidgetCheck)) {
    return false;
  }
  return true;
}

NodeSection*
SubControl::sectionIfIn(QPoint pos)
{
  auto section = Node::sectionIfIn(pos);
  if (section->type == Name) {
    section->type = SubControlName;
    return section;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = node_ptr->editor->fontMetrics();
  int top, bottom;
  QTextCursor c = cursor();
  rect = node_ptr->editor->cursorRect(c);
  top = rect.y();
  bottom = top + rect.height();
  left = rect.x();
  right = left + fm.horizontalAdvance("::");
  if (x >= left && x <= right && y >= top && y < bottom) {
    section->type = SubControlMarker;
    return section;
  }

  for (auto state : *(m_pseudoStates)) {
    section = state->sectionIfIn(pos);
    if (section->type == PseudoStateName) {
      section->type = SubControlPseudoStateName;
      return section;
    } else if (section->type == PseudoStateMarker) {
      section->type = SubControlPseudoStateMarker;
      return section;
    }
  }
  return section;
}

int
SubControl::pointWidth(const QString& text) const
{
  auto s = text;
  auto charSize = length() - 2; // remove marker

  for (auto state : *(m_pseudoStates)) {
    charSize -= state->length();
  }

  for (int i = 0; i < charSize; i++) {
    s += " ";
  }
  s += name();

  auto fm = node_ptr->editor->fontMetrics();
  return fm.horizontalAdvance(s);
}

PseudoState::PseudoState(QTextCursor markerCursor,
                         QTextCursor nameCursor,
                         const QString& name,
                         StylesheetEditor* editor)
  : MarkerBase(markerCursor, nameCursor, name, editor)
{
  m_marker = ":";
  if (!markerCursor.isNull()) {
    setCheck(PseudoStateMarkerCheck);
  }
}

bool
PseudoState::hasMarker()
{
  return (checks().testFlag(PseudoStateMarkerCheck));
}

bool
PseudoState::isFuzzy() const
{
  return (checks().testFlag(FuzzyPseudoStateCheck));
}

bool
PseudoState::isValid() const
{
  if (node_ptr->state.testFlag(FuzzyPseudoStateCheck) ||
      !node_ptr->state.testFlag(PseudoStateMarkerCheck)) {
    return false;
  }
  return true;
}

bool
PseudoState::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = node_ptr->editor->cursorRect(QTextCursor(node_ptr->cursor));
  auto left = rect.x();
  auto width = pointWidth(node_ptr->name);
  auto height = pointHeight();
  auto right = left + width;
  auto top = rect.y();
  auto bottom = top + height;

  if (x >= left && x <= right && y >= top && y <= bottom) {
    return true;
  }
  return false;
}

NodeSection*
PseudoState::sectionIfIn(QPoint pos)
{
  auto section = Node::sectionIfIn(pos);
  if (section->type == SectionType::Name) {
    section->type = SectionType::PseudoStateName;
    return section;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = node_ptr->editor->fontMetrics();
  int top, bottom;
  QTextCursor c = cursor();
  rect = node_ptr->editor->cursorRect(c);
  top = rect.y();
  bottom = top + rect.height();
  left = rect.x();
  right = left + fm.horizontalAdvance(":");
  if (x >= left && x <= right && y >= top && y < bottom) {
    section->type = PseudoStateMarker;
    return section;
  }
  return section;
}

int
PseudoState::pointWidth(const QString& text) const
{
  auto s = text;
  auto charSize = length() - 1; // remove marker

  for (int i = 0; i < charSize; i++) {
    s += " ";
  }
  s += name();

  auto fm = node_ptr->editor->fontMetrics();
  return fm.horizontalAdvance(s);
}
