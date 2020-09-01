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
#include <cstdarg>

namespace StylesheetParser {

class Node : public QObject
{

public:
  enum Type
  {
    NodeType,
    StringNodeType,
    CharNodeType,
    ColonNodeType,
    NameType,
    ValueType,
    WidgetType,
    SubControlType,
    SubControlMarkerType,
    PseudoStateType,
    PseudoStateMarkerType,
    SemiColonNodeType,
    StartBraceNodeType,
    EndBraceNodeType,
    NewlineNodeType,
  };
  explicit Node(int start, QObject* parent, Type type = NodeType);

  virtual int start() const;
  virtual int end() const;
  virtual int length() const;

  Type type() const;

protected:
  int m_start;
  Type m_type;
};

class StringNode : public Node
{
public:
  explicit StringNode(const QString& value, int start, QObject* parent, Type type = StringNodeType);

  QString value() const;
  int end() const override;
  int length() const override;

protected:
  QString m_value;
};

class NameNode : public StringNode
{
  Q_OBJECT
public:
  explicit NameNode(const QString& name, int start, QObject* parent, Type type = NameType);
};

class ValueNode : public StringNode
{
  Q_OBJECT
public:
  explicit ValueNode(const QString& name, int start, QObject* parent, Type type = ValueType);
};

//class NameValueNode : public Node
//{
//public:
//   NameValueNode(const QString& name,
//                 const QString& value,
//                 int start,
//                 QObject* parent);

//   QString name() const;
//   QString value() const;

//private:
//   QString m_name, m_value;
//};

class WidgetNode : public StringNode
{
  Q_OBJECT
public:
  explicit WidgetNode(const QString& name, int start, QObject* parent, Type type = WidgetType);

  QStringList widgets() const;
  static bool contains(const QString& widget) {
    return m_widgets.contains(widget);
  }
  void addWidget(const QString& widget) {
    if (!m_widgets.contains(widget)) {
      m_widgets.append(widget);
    }
  }
  void removeWidget(const QString& widget) {
    m_widgets.removeAll(widget);
  }

protected:
  static QStringList m_widgets;

  static QStringList initialiseList();

};

class SubControlNode : public WidgetNode
{
  Q_OBJECT
public:
  explicit SubControlNode(const QString& name, int start, QObject* parent, Type type = SubControlType);

  void addSubControl(const QString& control, const QString& widget);
  void addSubControl(const QString& control, QStringList& widgets);
  void removeSubControl(const QString& control);

  static bool contains(const QString& name);

  QStringList possibleWidgets() const;

protected:
  QStringList m_possibleWidgets;

  static QMap<QString, QStringList> m_subControls;

  static QStringList addControls(int count, ...);
  static QMap<QString, QStringList> initialiseMap();
};

class PseudoStateNode : public WidgetNode
{
  Q_OBJECT
public:
  explicit PseudoStateNode(const QString& name, int start, QObject* parent, Type type = PseudoStateType);

  void addPseudoState(const QString& state);
  void removePseudoState(const QString& state);

  static bool contains(const QString& state);

protected:
  static QStringList m_pseudoStates;

  static QStringList initialiseList();
};

class CharNode : public Node
{
  Q_OBJECT
public:
  explicit CharNode(int start, QObject* parent, Type type =CharNodeType);

  int end() const override;
  int length() const override;
};

class ColonNode : public CharNode
{
  Q_OBJECT
public:
  explicit ColonNode(int start, QObject* parent, Type type = ColonNodeType);
};

class PseudoStateMarkerNode : public ColonNode
{
  Q_OBJECT
public:
  explicit PseudoStateMarkerNode(int start, QObject* parent, Type type = PseudoStateMarkerType);
};

class SubControlMarkerNode : public CharNode
{
  Q_OBJECT
public:
  explicit SubControlMarkerNode(int start, QObject* parent, Type type = SubControlMarkerType);

  int end() const override;
  int length() const override;
};

class SemiColonNode : public CharNode
{
  Q_OBJECT
public:
  explicit SemiColonNode(int start, QObject* parent, Type type = SemiColonNodeType);
};

class NewlineNode : public CharNode
{
  Q_OBJECT
public:
  explicit NewlineNode(int start, QObject* parent, Type type = NewlineNodeType);
};

class StartBraceNode : public CharNode
{
  Q_OBJECT
public:
  explicit StartBraceNode(int start, QObject* parent, Type type = StartBraceNodeType);
};

class EndBraceNode : public CharNode
{
  Q_OBJECT
public:
  explicit EndBraceNode(int start, QObject* parent, Type type = EndBraceNodeType);
};

} // end of StylesheetParser

#endif // NODE_H
