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
#include "stylesheetparser/node.h"

namespace StylesheetParser {

Node::Node(int start, QObject* parent)
   : QObject(parent)
   , m_start(start)
{}

int Node::start() const
{
   return m_start;
}

int Node::end() const
{
   return m_start;
}

int Node::length() const
{
   return 0;
}

StringNode::StringNode(const QString& value, int start, QObject* parent)
   : Node(start, parent)
   , m_value(value)
{}

QString StringNode::value() const
{
   return m_value;
}

int StringNode::end() const
{
   return m_start + m_value.length();
}

int StringNode::length() const
{
   return m_value.length();
}

NameNode::NameNode(const QString& name, int start, QObject* parent)
   : StringNode(name, start, parent)
{}

ValueNode::ValueNode(const QString& value, int start, QObject* parent)
   : StringNode(value, start, parent)
{}

//NameValueNode::NameValueNode(const QString& name,
//                             const QString& value,
//                             int start,
//                             QObject* parent)
//   : Node(start, parent)
//   , m_name(name)
//   , m_value(value)
//{}

//QString NameValueNode::name() const
//{
//   return m_name;
//}

//QString NameValueNode::value() const
//{
//   return m_value;
//}

QStringList WidgetNode::m_widgets = WidgetNode::initialiseList();

WidgetNode::WidgetNode(const QString&name, int start, QObject* parent)
   : StringNode(name, start, parent)
{

}

QStringList WidgetNode::widgets() const
{
   return m_widgets;
}

QStringList WidgetNode::initialiseList()
{
   QStringList list;
   list << "QAbstractScrollArea" << "QCheckBox" << "QColumnView" << "QComboBox"
        << "QDateEdit"
        << "QDateTimeEdit" << "QDialog" << "QDialogButtonBox" << "QDockWidget"
        << "QDoubleSpinBox"
        << "QFrame" << "QGroupBox" << "QHeaderView" << "QLabel" << "QLineEdit"
        << "QListView"
        << "QListWidget" << "QMainWindow" << "QMenu" << "QMenu" << "QMessageBox"
        << "QProgressBar" << "QPushButton" << "QRadioButton" << "QScrollBar"
        << "QSizeGrip"
        << "QSlider" << "QSpinBox" << "QSplitter" << "QStatusBar"
        << "QTabBar" << "QTabWidget" << "QTableView" << "QTableWidget" << "QTextEdit"
        << "QTimeEdit" << "QToolBar" << "QToolButton" << "QToolBox"
        << "QToolTip" << "QTreeView" << "QTreeWidget" << "QWidget";
   return list;
}


QMap<QString, QStringList> SubControlNode::m_subControls =
   SubControlNode::initialiseMap();

SubControlNode::SubControlNode(const QString& name, int start, QObject* parent)
   : WidgetNode(name, start, parent)
{
   m_possibleWidgets = m_subControls.value(name);
}

void SubControlNode::addSubControl(const QString& control, const QString& widget)
{
   if (!WidgetNode::contains(widget)) {
      WidgetNode::addWidget(widget);
   }

   m_subControls.insert(control, addControls(1, &widget))     ;
}

void SubControlNode::addSubControl(const QString& control, QStringList& widgets)
{
   for (auto widget : widgets) {
      if (!WidgetNode::contains(widget)) {
         WidgetNode::addWidget(widget);
      }
   }

   m_subControls.insert(control, widgets);
}

void SubControlNode::removeSubControl(const QString& control)
{
   if (m_subControls.contains(control)) {
      m_subControls.remove(control);
   }
}

bool SubControlNode::contains(const QString& name)
{
   return m_subControls.contains(name);
}

QStringList SubControlNode::possibleWidgets() const
{
   return m_widgets;
}

QStringList SubControlNode::addControls(int count, ...)
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

QMap<QString, QStringList> SubControlNode::initialiseMap()
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

QStringList PseudoStateNode::m_pseudoStates = PseudoStateNode::initialiseList();

PseudoStateNode::PseudoStateNode(const QString& name, int start, QObject* parent)
   : WidgetNode(name, start, parent)
{
}

void PseudoStateNode::addPseudoState(const QString& state)
{
   if (!m_pseudoStates.contains(state)) {
      m_pseudoStates.append(state);
   }
}

void PseudoStateNode::removePseudoState(const QString& state)
{
   m_pseudoStates.removeAll(state);
}

bool PseudoStateNode::contains(const QString& state)
{
   return m_pseudoStates.contains(state);
}

QStringList PseudoStateNode::initialiseList()
{
   QStringList list;
   list << "active" << "adjoins-item" << "alternate" << "bottom" << "checked"
        << "closable" << "closed" << "default" << "disabled" << "editable"
        << "edit-focus" << "enabled" << "exclusive" << "first" << "flat" << "floatable"
        << "focus" << "has-children" << "has-siblings" << "horizontal" << "hover"
        << "indeterminate" << "last" << "left" << "maximized" << "middle"
        << "minimized" << "movable" << "no-frame" << "non-exclusive"
        << "off" << "on" << "only-one" << "open" << "next-selected"
        << "pressed" << "previous-selected" << "read-only" << "right"
        << "selected" << "top" << "unchecked" << "vertical" << "window";
   return list;
}

CharMode::CharMode(int start, QObject* parent)
   : Node(start, parent)
{
}

int CharMode::end() const
{
   return m_start + 1;
}

int CharMode::length() const
{
   return 1;
}

ColonNode::ColonNode(int start, QObject* parent)
   : CharMode(start, parent)
{}

DoubleColonNode::DoubleColonNode(int start, QObject* parent)
   : Node(start, parent)
{

}

int DoubleColonNode::end() const
{
   return m_start + 2;
}

int DoubleColonNode::length() const
{
   return 2;
}

SemiColonNode::SemiColonNode(int start, QObject* parent)
   : CharMode(start, parent)
{}

StartBraceNode::StartBraceNode(int start, QObject* parent)
   : CharMode(start, parent)
{}

EndBraceNode::EndBraceNode(int start, QObject* parent)
   : CharMode(start, parent)
{}





} // end of StylesheetParser
