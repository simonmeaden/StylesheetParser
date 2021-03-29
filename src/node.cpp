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

#include "stylesheetedit/stylesheetedit.h"
#include "stylesheetedit_p.h"

NamedNode::NamedNode(const QString& name,
                     QTextCursor cursor,
                     StylesheetEditor* editor,
                     QObject* parent,
                     enum NodeType type)
  : Node(cursor, editor, parent, type)
  , m_name(name)
{}

NamedNode::NamedNode(const NamedNode& other)
  : Node(other)
{}

NamedNode::~NamedNode() {}

int
NamedNode::end() const
{
  return position() + length();
}

int
NamedNode::pointWidth(const QString& text) const
{
  auto fm = m_editor->fontMetrics();
  return fm.horizontalAdvance(text);
}

int
NamedNode::pointHeight() const
{
  auto fm = m_editor->fontMetrics();
  return fm.height();
}

Node::Node(QTextCursor cursor,
           StylesheetEditor* editor,
           QObject* parent,
           NodeType type)
  : QObject(parent)
  , m_cursor(cursor)
  , m_type(type)
  , m_editor(editor)
{}

QDebug
operator<<(QDebug debug, const Node& node)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "Pos : " << node.position();
  return debug;
}

QDebug
operator<<(QDebug debug, const NamedNode& node)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "Pos : " << node.position() << " : " << node.name();
  return debug;
}

Node::Node(const Node& other) {}

QTextCursor
Node::cursor() const
{
  return m_cursor;
}

void
Node::setCursor(QTextCursor cursor)
{
  m_cursor = cursor;
}

int
Node::position() const
{
  return m_cursor.anchor();
}

int
Node::length() const
{
  return 0;
}

int
Node::end() const
{
  return position();
}

enum NodeType
Node::type() const
{
  return m_type;
}

QString
NamedNode::toString() const
{
  switch (m_type) {
    case WidgetType:
      return m_name;
      //    case StartBraceType:
      //      return "{";
      //    case EndBraceType:
      //      return "}";
    case NewlineType:
      return "\n";
    case PropertyType:
      return m_name;
    case CommentType:
      return m_name;
    case BadNodeType:
      return "";
  }

  return QString();
}

NodeStates
NamedNode::state() const
{
  return m_state;
}

void
NamedNode::setStateFlag(NodeState flag)
{
  m_state.setFlag(flag, true);
}

void
NamedNode::clearStateFlag(NodeState flag)
{
  m_state.setFlag(flag, false);
}

QString
NamedNode::name() const
{
  return m_name;
}

void
NamedNode::setName(const QString& value)
{
  m_name = value;
}

int
NamedNode::length() const
{
  return m_name.length();
}

bool
NamedNode::isIn(int pos)
{
  auto p = position();
  auto e = end();
  if (pos >= p && pos < e) {
    return true;
  }
  return false;
}

bool
NamedNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = m_editor->cursorRect(QTextCursor(m_cursor));
  auto left = rect.x();
  auto width = pointWidth(m_name);
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
NamedNode::sectionIfIn(QPoint pos)
{
  if (isIn(pos)) {
    return new NodeSection(this, SectionType::Name);
  }

  return new NodeSection(this);
}

WidgetNode::WidgetNode(const QString& name,
                       QTextCursor start,
                       StylesheetEditor* editor,
                       enum NodeState check,
                       QObject* parent,
                       enum NodeType type)
  : NamedNode(name, start, editor, parent, type)
{
  setWidgetCheck(check);
}

WidgetNode::WidgetNode(const WidgetNode& other)
  : NamedNode(other.name(),
              other.cursor(),
              other.m_editor,
              other.parent(),
              other.type())
{}

WidgetNode::~WidgetNode() {}

int
WidgetNode::length() const
{
  if (hasSubControl()) {
    return m_subcontrols->last()->cursor().anchor() - cursor().anchor() +
           m_subcontrols->length();
  } else {
    return name().length();
  }
}

bool
WidgetNode::isValid() const
{
  return (m_state.testFlag(NodeState::WidgetState) &&
          (!hasSubControl() || (hasSubControl() && doMarkersMatch())));
}

bool
WidgetNode::isNameValid() const
{
  return (m_state.testFlag(NodeState::WidgetState));
}

bool
WidgetNode::isNameFuzzy() const
{
  return (m_state.testFlag(NodeState::FuzzyWidgetState));
}

void
WidgetNode::setWidgetCheck(NodeState check)
{
  switch (check) {
    case NodeState::WidgetState: {
      m_state.setFlag(NodeState::WidgetState, true);
      m_state.setFlag(NodeState::FuzzyWidgetState, false);
      break;
    }
    case NodeState::FuzzyWidgetState: {
      m_state.setFlag(NodeState::FuzzyWidgetState, true);
      m_state.setFlag(NodeState::WidgetState, false);
      break;
    }
    case NodeState::SubControlState: {
      m_state.setFlag(NodeState::SubControlState, true);
      m_state.setFlag(NodeState::FuzzySubControlState, false);
      m_state.setFlag(NodeState::PseudostateState, false);
      m_state.setFlag(NodeState::FuzzyPseudostateState, false);
      break;
    }
    case NodeState::FuzzySubControlState: {
      m_state.setFlag(NodeState::SubControlState, false);
      m_state.setFlag(NodeState::FuzzySubControlState, true);
      m_state.setFlag(NodeState::PseudostateState, false);
      m_state.setFlag(NodeState::FuzzyPseudostateState, false);
      break;
    }
    case NodeState::PseudostateState: {
      m_state.setFlag(NodeState::SubControlState, false);
      m_state.setFlag(NodeState::FuzzySubControlState, false);
      m_state.setFlag(NodeState::PseudostateState, true);
      m_state.setFlag(NodeState::FuzzyPseudostateState, false);
      break;
    }
    case NodeState::FuzzyPseudostateState: {
      m_state.setFlag(NodeState::SubControlState, false);
      m_state.setFlag(NodeState::FuzzySubControlState, false);
      m_state.setFlag(NodeState::PseudostateState, false);
      m_state.setFlag(NodeState::FuzzyPseudostateState, true);
      break;
    }
  }
}

bool
WidgetNode::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = m_editor->cursorRect(QTextCursor(m_cursor));
  auto left = rect.x();
  auto width = pointWidth(m_name);
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
  auto section = NamedNode::sectionIfIn(pos);
  if (section->type == SectionType::Name) {
    section->type = SectionType::WidgetName;
    return section;
  }

  if (hasSubControl()) {
    for (auto subcontrol : *(m_subcontrols)) {
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

  return section;
}

QList<SubControl*>*
WidgetNode::subControls()
{
  return m_subcontrols;
}

SubControl*
WidgetNode::subControl(QPoint pos) const
{
  for (auto subcontrol : *(m_subcontrols)) {
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
    for (auto control : *(m_subcontrols)) {
      if (control->cursor() == cursor || control->nameCursor() == cursor) {
        return control;
      }
    }
  }
  return nullptr;
}

bool
WidgetNode::hasSubControls() const
{
  if (m_subcontrols && !m_subcontrols->isEmpty()) {
    return true;
  }
  return false;
}

void
WidgetNode::addSubControl(SubControl* control)
{
  if (!m_subcontrols)
    m_subcontrols = new QList<SubControl*>();

  if (!m_subcontrols->contains(control)) {
    m_subcontrols->append(control);
  } else {
    m_subcontrols->replace(m_subcontrols->indexOf(control), control);
  }
}

void
WidgetNode::addPseudoState(PseudoState* state)
{
  if (!m_pseudoStates)
    m_pseudoStates = new QList<PseudoState*>();

  if (!m_pseudoStates->contains(state)) {
    m_pseudoStates->append(state);
  } else {
    m_pseudoStates->replace(m_pseudoStates->indexOf(state), state);
  }
}

bool
WidgetNode::hasPseudoStates()
{
  if (m_pseudoStates && !m_pseudoStates->isEmpty()) {
    return true;
  }
  return false;
}

PseudoState*
WidgetNode::pseudoState(QPoint pos) const
{
  for (auto state : *(m_pseudoStates)) {
    if (state->isIn(pos)) {
      return state;
    }
  }
  return nullptr;
}

PseudoState*
WidgetNode::pseudoState(QTextCursor cursor) const
{
  if (m_pseudoStates) {
    for (auto state : *(m_pseudoStates)) {
      if (state->cursor() == cursor || state->cursor() == cursor) {
        return state;
      }
    }
  }
  return nullptr;
}

void
WidgetNode::setSubControlMarkerCursor(QTextCursor cursor)
{
  subControl(cursor)->setCursor(cursor);
  m_state.setFlag(NodeState::SubControlMarkerState, true);
}

void
WidgetNode::setPseudoStateMarkerCursor(QTextCursor cursor)
{
  pseudoState(cursor)->setCursor(cursor);
  m_state.setFlag(NodeState::PseudostateMarkerState, true);
}

IDSelector*
WidgetNode::idSelector()
{
  return m_idSelector;
}

void
WidgetNode::setIdSelector(IDSelector* selector)
{
  m_idSelector = selector;
}

bool
WidgetNode::hasIdSelector()
{
  return (m_idSelector);
}

bool
WidgetNode::doesMarkerMatch(enum NodeState type) const
{
  if (type == NodeState::PseudostateState ||
      type == NodeState::FuzzyPseudostateState) {
    if (m_state.testFlag(NodeState::PseudostateMarkerState)) {
      return true;
    }
  } else if (type == NodeState::SubControlState ||
             type == NodeState::FuzzySubControlState) {
    if (m_state.testFlag(NodeState::SubControlMarkerState)) {
      return true;
    }
  }
  return false;
}

bool
WidgetNode::doMarkersMatch() const
{
  return (doesMarkerMatch(NodeState::PseudostateState) ||
          doesMarkerMatch(NodeState::SubControlState));
}

bool
WidgetNode::isSubControlValid(QPoint pos) const
{
  if (hasSubControl()) {
    auto subcontrol = subControl(pos);
    auto checks = subcontrol->state();
    if (checks.testFlag(NodeState::SubControlState) &&
        !checks.testFlag(NodeState::BadSubControlForWidgetState)) {
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
    return (m_state.testFlag(NodeState::FuzzySubControlState));
  }
  return false;
}

bool
WidgetNode::isSubControlFuzzy(QPoint pos) const
{
  auto subcontrol = subControl(pos);
  if (subcontrol) {

    return (m_state.testFlag(NodeState::FuzzySubControlState));
  }
  return false;
}

bool
WidgetNode::isSubControlBad(QPoint pos) const
{
  auto subcontrol = subControl(pos);
  if (subcontrol) {
    return (subControl(pos)->state().testFlag(
      NodeState::BadSubControlForWidgetState));
  }
  return true;
}

bool
WidgetNode::isSubControl() const
{
  return (m_state.testFlag(NodeState::SubControlState) ||
          m_state.testFlag(NodeState::FuzzySubControlState));
}

bool
WidgetNode::isPseudoState() const
{
  return (m_state.testFlag(NodeState::PseudostateState) ||
          m_state.testFlag(NodeState::FuzzyPseudostateState));
}

bool
WidgetNode::hasSubControl() const
{
  return (m_subcontrols);
}

bool
WidgetNode::isExtensionMarkerCorrect()
{
  return ((m_state.testFlag(PseudostateState) &&
           m_state.testFlag(PseudostateMarkerState)) ||
          (m_state.testFlag(SubControlState) &&
           m_state.testFlag(SubControlMarkerState)));
}

bool
WidgetNodes::hasStartBrace() const
{
  return (m_state.testFlag(NodeState::StartBraceState));
}

void
WidgetNodes::setStartBraceCursor(QTextCursor cursor)
{
  m_state.setFlag(NodeState::StartBraceState, true);
  m_startBracePosition = cursor;
}

QTextCursor
WidgetNodes::startBraceCursor() const
{
  return m_startBracePosition;
}

int
WidgetNodes::startBracePosition() const
{
  return m_startBracePosition.anchor();
}

void
WidgetNodes::removeStartBrace()
{
  (m_state.setFlag(NodeState::StartBraceState, false));
}

bool
WidgetNodes::hasEndBrace() const
{
  return (m_state.testFlag(NodeState::EndBraceState));
}

void
WidgetNodes::setEndBraceCursor(QTextCursor cursor)
{
  m_state.setFlag(NodeState::EndBraceState, true);
  m_endBracePosition = cursor;
}

QTextCursor
WidgetNodes::endBraceCursor() const
{
  return m_endBracePosition;
}

int
WidgetNodes::endBracePosition() const
{
  return m_endBracePosition.anchor();
}

void
WidgetNodes::removeEndBrace()
{
  m_state.setFlag(NodeState::EndBraceState, false);
}

bool
WidgetNodes::hasMatchingBraces() const
{
  return (m_state.testFlag(NodeState::StartBraceState) &&
          m_state.testFlag(NodeState::EndBraceState));
}

void
WidgetNodes::addProperty(PropertyNode* property)
{
  m_properties.append(property);
}

PropertyNode*
WidgetNodes::property(int index) const
{
  return m_properties.at(index);
}

int
WidgetNodes::propertyCount() const
{
  return m_properties.size();
}

bool
WidgetNodes::isFinalProperty(PropertyNode* property) const
{
  auto index = m_properties.indexOf(property);
  return (index == m_properties.size() - 1);
}

void
WidgetNodes::addSeperatorCursor(QTextCursor cursor)
{
  m_widgetSeperators.append(cursor);
}

QList<QTextCursor>
WidgetNodes::seperators()
{
  return m_widgetSeperators;
}

bool
WidgetNodes::arePropertiesValid() const
{
  for (auto property : m_properties) {
    bool final = (property == m_properties.last());
    if (!property->isValid(final)) {
      return false;
    }
  }
  return true;
}

int
WidgetNodes::length() const
{
  auto start = position();
  return end() - start;
}

int
WidgetNodes::end() const
{
  int end = position();
  if (hasEndBrace()) {
    end = endBracePosition();
  } else if (!m_properties.isEmpty()) {
    end = m_properties.last()->end();
  } else if (hasStartBrace()) {
    end = startBracePosition();
  } else if (!m_widgets.isEmpty()) {
    end = m_widgets.last()->end();
  }
  return end;
}

bool
WidgetNodes::isIn(int pos)
{
  if (pos > position() && pos < end()) {
    return true;
  }
  return false;
}

bool
WidgetNodes::isIn(QPoint pos)
{
  for (auto widget : m_widgets) {
    if (widget->isIn(pos)) {
      return true;
    }
  }
  for (auto property : m_properties) {
    if (property->isIn(pos)) {
      return true;
    }
  }

  if (isInStartBrace(pos)) {
    return true;
  }

  if (isInEndBrace(pos)) {
    return true;
  }

  return false;
}

NodeSection*
WidgetNodes::sectionIfIn(QPoint pos)
{
  NodeSection* section = nullptr;
  for (auto widget : m_widgets) {
    section = widget->sectionIfIn(pos);
    if (section) {
      return section;
    }
  }

  for (auto property : m_properties) {
    section = property->sectionIfIn(pos);
    if (section) {
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
      return section;
    }
  }

  if (isInStartBrace(pos)) {
    section = new NodeSection(this);
    section->type = SectionType::WidgetStartBrace;
    return section;
  }

  if (isInEndBrace(pos)) {
    section = new NodeSection(this);
    section->type = SectionType::WidgetEndBrace;
    return section;
  }

  return nullptr;
}

bool
WidgetNodes::isInStartBrace(QPoint pos)
{
  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = m_editor->fontMetrics();
  int top, bottom;
  QTextCursor cursor;

  if (hasStartBrace()) {
    cursor = startBraceCursor();
    rect = m_editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance("{");
    if (x >= left && x <= right && y >= top && y < bottom) {
      return true;
    }
  }
  return false;
}

bool
WidgetNodes::isInEndBrace(QPoint pos)
{
  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = m_editor->fontMetrics();
  int top, bottom;
  QTextCursor cursor;

  if (hasEndBrace()) {
    cursor = endBraceCursor();
    rect = m_editor->cursorRect(cursor);
    top = rect.y();
    bottom = top + rect.height();
    left = rect.x();
    right = left + fm.horizontalAdvance("}");
    if (x >= left && x <= right && y >= top && y < bottom) {
      return true;
    }
  }
  return false;
}

PropertyNode::PropertyNode(const QString& name,
                           QTextCursor start,
                           StylesheetEditor* editor,
                           NodeState check,
                           QObject* parent,
                           enum NodeType type)
  : NamedNode(name, start, editor, parent, type)
{
  setPropertyNameCheck(check);
}

PropertyNode::PropertyNode(const PropertyNode& other)
  : NamedNode(other.name(),
              other.cursor(),
              other.m_editor,
              other.parent(),
              other.type())
{}

PropertyNode::~PropertyNode()
{
  qDeleteAll(m_valueStatus);
}

void
PropertyNode::setWidgetNodes(WidgetNodes* widget)
{
  m_widgetnodes = widget;
}

bool
PropertyNode::hasWidgetNodes()
{
  return (m_widgetnodes);
}

QStringList
PropertyNode::values() const
{
  return m_values;
}

QList<NodeState>
PropertyNode::state() const
{
  return m_checks;
}

QList<QTextCursor>
PropertyNode::valueCursors() const
{
  return m_cursors;
}

void
PropertyNode::setValueCursor(int index, QTextCursor cursor)
{
  if (index >= 0 && index < m_cursors.length()) {
    m_cursors.replace(index, cursor);
  }
}

// QList<PropertyValueState>
// PropertyNode::valueStatus() const
//{
//  return m_valueStatus;
//}

PropertyStatus*
PropertyNode::valueStatus(int index) const
{
  if (index >= 0 && index < m_valueStatus.size())
    return m_valueStatus.at(index);
  return nullptr;
}

void
PropertyNode::setValues(const QStringList& values)
{
  m_values = values;
}

void
PropertyNode::setValue(int index, const QString& value)
{
  if (index >= 0 && index < m_values.length()) {
    m_values.replace(index, value);
  }
}

void
PropertyNode::setChecks(const QList<NodeState>& checks)
{
  m_checks = checks;
}

void
PropertyNode::setStateFlag(int index, NodeState check)
{
  if (index >= 0 && index < m_checks.length()) {
    m_checks.replace(index, check);
  }
}

NodeState
PropertyNode::check(int index)
{
  if (index >= 0 && index < m_values.size())
    return m_checks.at(index);
  return BadNodeState;
}

void
PropertyNode::setValueCursors(const QList<QTextCursor>& positions)
{
  m_cursors = positions;
}

int
PropertyNode::valuePosition(int index)
{
  if (index >= 0 && index < m_cursors.size())
    return m_cursors.at(index).anchor();
  return -1;
}

void
PropertyNode::addValue(const QString& value,
                       NodeState check,
                       QTextCursor cursor,
                       PropertyStatus* status)
{
  m_values.append(value);
  m_checks.append(check);
  m_cursors.append(cursor);
  m_valueStatus.append(status);
}

QString
PropertyNode::value(int index)
{
  if (index >= 0 && index < m_values.size())
    return m_values.at(index);
  return QString();
}

// bool
// PropertyNode::setBadCheck(PropertyValueCheck check, int index)
//{
//  if (index == -1 && !m_checks.isEmpty()) {
//    m_checks[m_checks.length() - 1] = check;
//    return true;

//  } else if (index >= 0 && index < m_checks.length()) {
//    m_checks[index] = check;
//    return true;
//  }

//  return false;
//}

int
PropertyNode::count()
{
  return m_cursors.size();
}

bool
PropertyNode::isValueValid(int index)
{
  if (index >= 0 && index < count()) {
    return m_checks.at(index) != BadNodeState;
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
  if (hasPropertyEndMarker()) {
    return m_endMarkerCursor.anchor() - position();
  } else if (m_cursors.isEmpty()) {
    if (!hasPropertyMarker()) {
      return m_name.length();
    } else {
      return propertyMarkerCursor().anchor() + 1;
    }
  } else {
    auto value = m_values.last();
    auto status = m_valueStatus.last();
    return m_cursors.last().anchor() + position() + status->offset +
           value.length();
  }

  return 0;
}

void
PropertyNode::setPropertyNameCheck(NodeState check)
{
  if (check == ValidNameState) {
    m_propertyState.setFlag(check, true);
    m_propertyState.setFlag(FuzzyPropertyState, false);
  } else if (check == FuzzyPropertyState) {
    m_propertyState.setFlag(ValidNameState, false);
    m_propertyState.setFlag(check, true);
  } else if (check == BadNodeState) {
    m_propertyState.setFlag(ValidNameState, false);
    m_propertyState.setFlag(FuzzyPropertyState, false);
  }
}

bool
PropertyNode::hasPropertyMarker() const
{
  return m_propertyState.testFlag(NodeState::PropertyMarkerState);
}

void
PropertyNode::setPropertyMarker(bool exists)
{
  m_propertyState.setFlag(NodeState::PropertyMarkerState, exists);
}

bool
PropertyNode::isValidPropertyName() const
{
  return (m_propertyState.testFlag(NodeState::ValidNameState));
}

bool
PropertyNode::isFuzzyName() const
{
  return (m_propertyState.testFlag(NodeState::FuzzyPropertyState));
}

bool
PropertyNode::isValid(bool finalProperty)
{
  if (m_widgetnodes) {
    if (m_widgetnodes->isFinalProperty(this)) {
      return isValidPropertyName() && hasPropertyMarker();
    }
  }

  if (!hasPropertyEndMarker() && finalProperty) {
    return isValidPropertyName() && hasPropertyMarker();
  }

  return m_propertyState.testFlag(ValidNameState);
}

QTextCursor
PropertyNode::propertyMarkerCursor() const
{
  return m_propertyMarkerCursor;
}

int
PropertyNode::propertyMarkerPosition() const
{
  return m_propertyMarkerCursor.anchor();
}

void
PropertyNode::setPropertyMarkerCursor(QTextCursor position)
{
  m_propertyMarkerCursor = position;
}

bool
PropertyNode::hasPropertyEndMarker() const
{
  return m_propertyState.testFlag(NodeState::PropertyEndMarkerState);
}

void
PropertyNode::setPropertyEndMarker(bool exists)
{
  m_propertyState.setFlag(NodeState::PropertyEndMarkerState, exists);
}

QTextCursor
PropertyNode::propertyEndMarkerCursor() const
{
  return m_endMarkerCursor;
}

int
PropertyNode::propertyEndMarkerPosition() const
{
  return m_endMarkerCursor.anchor();
}

void
PropertyNode::setPropertyEndMarkerCursor(QTextCursor position)
{
  m_endMarkerCursor = position;
}

NodeSection*
PropertyNode::sectionIfIn(QPoint pos)
{
  // this just checks the property name.
  auto isin = NamedNode::sectionIfIn(pos);
  if (isin->type == SectionType::Name) {
    isin->type = SectionType::PropertyName;
    return isin;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = m_editor->fontMetrics();
  int top, bottom;
  QTextCursor cursor;

  // check marker;
  if (hasPropertyMarker()) {
    cursor = propertyMarkerCursor();
    rect = m_editor->cursorRect(cursor);
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
    rect = m_editor->cursorRect(cursor);
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
    rect = m_editor->cursorRect(cursor);
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

QList<PartialType>
PropertyNode::sectionIfIn(int start, int end)
{
  QList<PartialType> partials;
  int valueEnd;
  if (start >= position() && start < NamedNode::end()) {
    PartialType pt;
    pt.start = start;
    pt.type = PartialType::Name;
    valueEnd = position() + m_name.length();
    if (end >= valueEnd) {
      pt.end = start + m_name.length();
    } else {
      pt.end = end;
    }
    partials.append(pt);
  }

  if (hasPropertyMarker()) {
    PartialType pt;
    auto pos = m_propertyMarkerCursor.anchor();
    if (pos >= start && pos < end) {
      pt.start = pos;
      pt.end = pos + 1;
      pt.type = PartialType::Marker;
      partials.append(pt);
    }
  }

  for (int i = 0; i < m_values.count(); i++) {
    auto offset = m_cursors.at(i).anchor();
    if (end < offset)
      break;
    auto value = m_values.at(i);
    valueEnd = offset + value.length();
    PartialType pt;
    if (start <= offset) {
      if (end > offset) {
        pt.start = offset;
      }
    } else if (start < valueEnd) {
      pt.start = start;
    }
    if (end >= start) {
      if (end < valueEnd) {
        pt.end = end;
      } else {
        pt.end = valueEnd;
      }
    }
    if (start != -1 && end != -1) {
      pt.type = PartialType::Value;
      partials.append(pt);
    }
  }

  if (hasPropertyEndMarker()) {
    PartialType pt;
    auto pos = m_endMarkerCursor.anchor();
    if (pos >= start && pos < end) {
      pt.start = pos;
      pt.end = pos + 1;
      pt.type = PartialType::EndMarker;
      partials.append(pt);
    }
  }

  return partials;
}

CommentNode::CommentNode(QTextCursor start,
                         StylesheetEditor* editor,
                         QObject* parent,
                         enum NodeType type)
  : Node(start, editor, parent, type)
// TODO set actual comment check value
{}

CommentNode::CommentNode(const CommentNode& other)
  : Node(other.cursor(),
         other.m_editor, // TODO set actual comment check value
         other.parent(),
         other.type())
{}

CommentNode::~CommentNode() {}

void
CommentNode::append(QChar c)
{
  m_text.append(c);
}

void
CommentNode::append(QString text)
{
  m_text.append(text);
}

int
CommentNode::end() const
{
  return m_cursor.anchor() + m_text.length();
}

int
CommentNode::length() const
{
  if (hasEndComment()) {
    return endCommentCursor().anchor() - position() + 2;
  } else if (m_text.length() > 0) {
    return cursor().anchor() + m_text.length() - position();
  } else {
    return 2; // length of start comment
  }
}

QTextCursor
CommentNode::textCursor() const
{
  return m_textCursor;
}

void
CommentNode::setTextCursor(QTextCursor cursor)
{
  m_textCursor = cursor;
}

int
CommentNode::textPosition() const
{
  return m_textCursor.anchor();
}

bool
CommentNode::hasEndComment() const
{
  return m_endCommentExists;
}

void
CommentNode::setEndCommentExists(bool exists)
{
  m_endCommentExists = exists;
}

QTextCursor
CommentNode::endCommentCursor() const
{
  return m_endCursor;
}

void
CommentNode::setEndCommentCursor(QTextCursor cursor)
{
  m_endCursor = cursor;
}

NodeSection*
CommentNode::sectionIfIn(QPoint pos)
{
  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = m_editor->fontMetrics();
  int top, bottom;
  NodeSection* isin = new NodeSection(this);

  // check marker;
  QTextCursor s(m_editor->document());
  s.setPosition(position());
  rect = m_editor->cursorRect(s);
  top = rect.y();
  bottom = top + rect.height();
  left = rect.x();
  s.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, length());
  rect = m_editor->cursorRect(s);
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
  : NamedNode("\n", start, editor, parent, type)
{}

MarkerBase::MarkerBase(QTextCursor markerCursor,
                       QTextCursor nameCursor,
                       const QString& name,
                       StylesheetEditor* editor,
                       QObject* parent,
                       NodeType type)
  : NamedNode(name, nameCursor, editor, parent, type)
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
  return NamedNode::cursor();
}

void
MarkerBase::setCursor(QTextCursor markerCursor)
{
  m_markerCursor = markerCursor;
}

void
MarkerBase::setNameCursor(QTextCursor cursor)
{
  NamedNode::setCursor(cursor);
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

bool
MarkerBase::isFuzzy() const
{
  return false;
}

int
MarkerBase::length() const
{
  return (m_markerCursor.position() - NamedNode::position() + name().length());
}

ControlBase::ControlBase(QTextCursor markerCursor,
                         QTextCursor nameCursor,
                         const QString& name,
                         StylesheetEditor* editor,
                         QObject* parent,
                         enum NodeType type)
  : MarkerBase(markerCursor, nameCursor, name, editor, parent, type)
{
  m_marker = "::";
  if (!markerCursor.isNull()) {
    setStateFlag(NodeState::SubControlMarkerState);
  }
}

QList<PseudoState*>*
ControlBase::pseudoStates() const
{
  return m_pseudoStates;
}

bool
ControlBase::hasPseudoStates()
{
  if (m_pseudoStates && !m_pseudoStates->isEmpty()) {
    return true;
  }
  return false;
}

PseudoState*
ControlBase::pseudoState(QTextCursor cursor)
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
ControlBase::addPseudoState(PseudoState* state)
{
  if (!m_pseudoStates)
    m_pseudoStates = new QList<PseudoState*>();

  if (!m_pseudoStates->contains(state)) {
    m_pseudoStates->append(state);
  } else {
    m_pseudoStates->replace(m_pseudoStates->indexOf(state), state);
  }
}

bool
ControlBase::hasMarker()
{
  return (state().testFlag(SubControlMarkerState));
}

int
ControlBase::length() const
{
  if (m_pseudoStates && !m_pseudoStates->isEmpty()) {
    auto state = m_pseudoStates->last();
    return (state->position() + state->length() - position());
  } else {
    return MarkerBase::length();
  }
}

bool
ControlBase::isFuzzy() const
{
  return (state().testFlag(FuzzySubControlState));
}

bool
ControlBase::isValid() const
{
  if (m_state.testFlag(FuzzySubControlState) ||
      !m_state.testFlag(SubControlMarkerState) ||
      m_state.testFlag(BadSubControlForWidgetState)) {
    return false;
  }
  return true;
}

NodeSection*
ControlBase::sectionIfIn(QPoint pos)
{
  auto section = NamedNode::sectionIfIn(pos);
  if (section->type == Name) {
    section->type = SubControlName;
    return section;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = m_editor->fontMetrics();
  int top, bottom;
  QTextCursor c = cursor();
  rect = m_editor->cursorRect(c);
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
ControlBase::pointWidth(const QString& text) const
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

  auto fm = m_editor->fontMetrics();
  return fm.horizontalAdvance(s);
}

PseudoState::PseudoState(QTextCursor markerCursor,
                         QTextCursor nameCursor,
                         const QString& name,
                         StylesheetEditor* editor,
                         QObject* parent,
                         NodeType type)
  : MarkerBase(markerCursor, nameCursor, name, editor, parent, type)
{
  m_marker = ":";
  if (!markerCursor.isNull()) {
    setStateFlag(PseudostateMarkerState);
  }
}

bool
PseudoState::hasMarker()
{
  return (state().testFlag(PseudostateMarkerState));
}

bool
PseudoState::isFuzzy() const
{
  return (state().testFlag(FuzzyPseudostateState));
}

bool
PseudoState::isValid() const
{
  if (m_state.testFlag(PseudostateState) &&
      m_state.testFlag(PseudostateMarkerState)) {
    return true;
  }
  return false;
}

bool
PseudoState::isIn(QPoint pos)
{
  auto x = pos.x();
  auto y = pos.y();
  auto rect = m_editor->cursorRect(QTextCursor(m_cursor));
  auto left = rect.x();
  auto width = pointWidth(m_name);
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
  auto section = NamedNode::sectionIfIn(pos);
  if (section->type == SectionType::Name) {
    section->type = SectionType::PseudoStateName;
    return section;
  }

  QRect rect;
  auto x = pos.x();
  auto y = pos.y();
  int left, right;
  auto fm = m_editor->fontMetrics();
  int top, bottom;
  QTextCursor c = cursor();
  rect = m_editor->cursorRect(c);
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

  auto fm = m_editor->fontMetrics();
  return fm.horizontalAdvance(s);
}

SubControl::SubControl(QTextCursor markerCursor,
                       QTextCursor nameCursor,
                       const QString& name,
                       StylesheetEditor* editor,
                       QObject* parent,
                       enum NodeType type)
  : ControlBase(markerCursor, nameCursor, name, editor, parent, type)
{
  m_marker = "::";
  if (!markerCursor.isNull()) {
    setStateFlag(IdSelectorMarkerState);
  }
}

IDSelector::IDSelector(QTextCursor markerCursor,
                       QTextCursor nameCursor,
                       const QString& name,
                       StylesheetEditor* editor,
                       QObject* parent,
                       enum NodeType type)
  : ControlBase(markerCursor, nameCursor, name, editor, parent, type)
{
  m_marker = "#";
  if (!markerCursor.isNull()) {
    setStateFlag(IdSelectorMarkerState);
  }
}

bool
IDSelector::isValid() const
{
  if (m_marker == "#" && m_name.length() > 0) {
    return true;
  }
  return false;
}

bool
PropertyNode::isFinalProperty()
{
  return m_isFinalProperty;
}
