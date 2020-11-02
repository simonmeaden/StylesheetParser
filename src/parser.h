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

#include "common.h"
#include "parserstate.h"

class StylesheetEdit;
class DataStore;
class Node;
class PropertyNode;
class StartBraceNode;
class EndBraceNode;
class WidgetNode;

struct ParserData
{
  QMap<QTextCursor, Node*> nodes;
  QList<StartBraceNode*> startbraces;
  QList<EndBraceNode*> endbraces;
  int braceCount;
  bool /*startComment,*/ manualMove, suggestion;
  QTextCursor currentCursor;
  Node* currentWidget;
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
  explicit Parser(StylesheetEdit* parent = nullptr);
  Parser(const Parser& other);

  ~Parser();

  Parser& operator=(const Parser& other);

  void parseInitialText(const QString& text, int pos = 0);

  QMap<QTextCursor, Node*> nodes() const;
  StylesheetData* getStylesheetProperty(const QString& sheet, int& pos);
  void handleDocumentChanged(int pos, int charsRemoved, int charsAdded);
  void handleCursorPositionChanged(QTextCursor textCursor);
  void handleMouseClicked(const QPoint& pos);
  QTextCursor currentCursor() const;
  void setCurrentCursor(const QTextCursor& currentCursor);

  int maxSuggestionCount() const;
  void setMaxSuggestionCount(int maxSuggestionCount);

  bool manualMove() const;
  void setManualMove(bool manualMove);

  QMenu* contextMenu() const;

  void handleSuggestion(QAction* act);

  QPair<NodeSectionType, int> nodeForPoint(const QPoint& pos,
                                           Node **nNode);

signals:
  void finished();
  void rehighlight();

private:
  StylesheetEdit* m_editor;
  ParserData* d_ptr;
  DataStore* m_datastore;
  QMenu *m_contextMenu, *m_suggestionsMenu;
  QAction *m_addPropertyMarkerAct, *m_formatAct;

  int parsePropertyWithValues(PropertyNode* property,
                              const QString& text,
                              int start,
                              int& pos,
                              QString& block,
                              WidgetNode *widget=nullptr);
  void parseComment(const QString& text, int& pos);

  QString findNext(const QString& text, int& pos);
  void skipBlanks(const QString& text, int& pos);

  QTextCursor getCursorForNode(int position);
  CursorData getNodeAtCursor(QTextCursor cursor);
  CursorData getNodeAtCursor(int position);
  void nodeAtCursorPosition(CursorData* data, int position);
  Node* nextNode(QTextCursor cursor);
  Node* previousNode(QTextCursor cursor);

  WidgetNode *stashWidget(int position, const QString& block, bool valid = true);
  void stashBadNode(int position,
                    const QString& block,
                    ParserState::Error error);
//  void stashBadSubControlMarkerNode(int position, ParserState::Error error);
//  void stashBadPseudoStateMarkerNode(int position, ParserState::Error error);
//  void stashPseudoState(int position, const QString& block, bool valid = true);
//  void stashSubControl(int position, const QString& block, bool valid = true);
  EndBraceNode* stashEndBrace(int position);
  StartBraceNode* stashStartBrace(int position);
//  void stashPseudoStateMarker(int position);
//  void stashSubControlMarker(int position);
//  void stashPropertyEndNode(int position, Node** endnode);
//  void stashPropertyEndMarkerNode(int position, Node** endnode);

  void updatePropertyValues(int pos,
                            PropertyNode* property,
                            int charsAdded,
                            int charsRemoved,
                            const QString& newValue);

  QMenu* createContextMenu();
  void updateContextMenu(QMap<int, QString> matches,
                         Node *nNode,
                         const QPoint& pos);
  void updatePropertyContextMenu(QMap<int, QString> matches,
                                 PropertyNode* property,
                                 const QPoint& pos);
  void updatePropertyValueContextMenu(QMultiMap<int, QString> matches,
                                      PropertyNode* nNode,
                                      const QString& valueName,
                                      const QPoint& pos);
  void updatePropertyValueContextMenu(
    QMultiMap<int, QPair<QString, QString>> matches,
    PropertyNode* nNode,
    const QString& valueName,
    const QPoint& pos);
  void updateMenu(QMap<int, QString> matches,
                  Node *nNode,
                  const QPoint& pos);
  QList<int> reverseLastNValues(QMultiMap<int, QString> matches);
  QList<QPair<QString, QString>> sortLastNValues(
    QMultiMap<int, QPair<QString, QString>> matches);
  void actionPropertyNameChange(PropertyNode* property,
                                const QString& newName);
  void actionPropertyValueChange(PropertyNode* property,
                                 const PropertyStatus& status,
                                 const QString& newName);
  void stepBack(int &pos, const QString &block);
  enum NodeType checkType(const QString &block);

};

#endif // PARSER_H
