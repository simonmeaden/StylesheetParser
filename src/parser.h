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
#ifndef PARSER_H
#define PARSER_H

#include <QMenu>
#include <QObject>
#include <QPoint>
#include <QStack>
#include <QTextCursor>
#include <QWidgetAction>

#include "common.h"
#include "parserstate.h"

class StylesheetEditor;
class DataStore;
class WidgetNode;
class PropertyNode;
class StartBraceNode;
class EndBraceNode;
class WidgetNode;
class NewlineNode;

class IconLabel : public QWidget
{
public:
  IconLabel(const QIcon& icon, const QString& text, QWidget* parent);
};

struct ParserData
{
  QMap<QTextCursor, WidgetNode*> nodes;
  QList<StartBraceNode*> startbraces;
  QList<EndBraceNode*> endbraces;
  int braceCount;
  bool /*startComment,*/ manualMove, suggestion;
  QTextCursor currentCursor;
  WidgetNode* currentWidget;
  int maxSuggestionCount;
  QStack<StartBraceNode*> braceStack;

  ParserData()
    : braceCount(0)
    //    , startComment(false)
    , manualMove(false)
    , maxSuggestionCount(30)
    , suggestion(false)
  {}
};

class Parser : public QObject
{
  Q_OBJECT
public:
  explicit Parser(DataStore* datastore,
                  StylesheetEditor* editor,
                  QObject* parent = nullptr);
  Parser(const Parser&);

  ~Parser();

  Parser& operator=(const Parser&);

  void parseInitialText(const QString& text);

  StylesheetData* getStylesheetProperty(const QString& sheet, int& pos);
  void handleDocumentChanged(int pos, int charsRemoved, int charsAdded);
  void handleCursorPositionChanged(QTextCursor textCursor);
  QMenu* handleMouseClicked(const QPoint& pos);
  QTextCursor currentCursor() const;
  void setCurrentCursor(const QTextCursor& currentCursor);

  int maxSuggestionCount() const;
  void setMaxSuggestionCount(int maxSuggestionCount);

  void handleSuggestions(QAction* act);

  void nodeForPoint(const QPoint& pos, NodeSection** nodeSection);

  bool showLineMarkers() const;
  void setShowLineMarkers(bool showLineMarkers);

signals:
  void finished();
  void parseComplete(bool initialsed = false);
  void rehighlight();
  void rehighlightBlock(const QTextBlock& block);

  void setBraceCount(int);

private:
  StylesheetEditor* m_editor;
  DataStore* m_datastore;
  //  QAction *m_addPropertyMarkerAct,
  //    *m_addPropertyEndMarkerAct;
  bool m_showLineMarkers;

  void parsePropertyWithValues(PropertyNode* property,
                               const QString& text,
                               int start,
                               int& pos,
                               QString& block);
  void parseComment(const QString& text, int start, int& pos);

  QString findNext(const QString& text, int& pos);
  void skipBlanks(const QString& text, int& pos);

  QTextCursor getCursorForPosition(int position);
  CursorData getNodeAtCursor(QTextCursor cursor);
  CursorData getNodeAtCursor(int position);
  void nodeAtCursorPosition(CursorData* data, int position);

  WidgetNode* stashWidget(int position,
                          const QString& block,
                          bool valid = true);
  void stashBadNode(int position,
                    const QString& block,
                    ParserState::Error error);
  void stashNewline(int position);
  void stashEndBrace(int position);
  void stashStartBrace(int position);

  void updateContextMenu(QMap<int, QString> matches,
                         WidgetNode* node,
                         const QPoint& pos,
                         QMenu** suggestionsMenu);
  void updatePropertyContextMenu(PropertyNode* property,
                                 const QPoint& pos,
                                 QMenu** suggestionsMenu,
                                 QMap<int, QString> matches);
  void updateValidPropertyValueContextMenu(QMultiMap<int, QString> matches,
                                           PropertyNode* property,
                                           const QString& valueName,
                                           const QPoint& pos,
                                           QMenu** suggestionsMenu);
  void updateInvalidPropertyValueContextMenu(QMultiMap<int, QString> matches,
                                             PropertyNode* property,
                                             const QString& valueName,
                                             const QPoint& pos,
                                             QMenu** suggestionsMenu);
  void updateInvalidNameAndPropertyValueContextMenu(
    QMultiMap<int, QPair<QString, QString>> matches,
    PropertyNode* nNode,
    const QString& valueName,
    const QPoint& pos,
    QMenu** suggestionsMenu);
  void updateMenu(QMap<int, QString> matches,
                  Node* nNode,
                  const QPoint& pos,
                  QMenu** suggestionsMenu,
                  SectionType type,
                  const QString& oldName = QString());
  QList<int> reverseLastNValues(QMultiMap<int, QString> matches);
  QList<QPair<QString, QString>> sortLastNValues(
    QMultiMap<int, QPair<QString, QString>> matches);
  void actionPropertyNameChange(PropertyNode* property, const QString& newName);
  void actionPropertyValueChange(PropertyNode* property,
                                 const QString& oldName,
                                 const QString& newName);
  void actionPropertyMarker(PropertyNode* property);
  void actionPropertyEndMarker(PropertyNode* property);
  void stepBack(int& pos, const QString& block);
  enum NodeType checkType(const QString& block,
                          PropertyNode* property = nullptr) const;
  QWidgetAction* getWidgetAction(const QIcon& icon,
                                 const QString& text,
                                 QMenu* suggestionsMenu);

  void updateTextChange(QTextCursor& cursor,
                        const QString& oldName,
                        const QString& newName);
  void setMenuData(QAction* act,
                   Node* property,
                   QPoint pos,
                   SectionType type,
                   const QString& oldName = QString());
};

#endif // PARSER_H
