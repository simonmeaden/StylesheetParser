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
#include "stylesheetparser/parser.h"
#include "stylesheetparser/node.h"
#include "stylesheetparser/parserstate.h"
#include "stylesheetparser/stylesheetedit.h"

namespace StylesheetEditor {

Parser::Parser(StylesheetEdit* editor, DataStore* datastore, QObject* parent)
  : QObject(parent)
  , m_editor(editor)
  , m_nodes(new NodeList())
  , m_datastore(datastore)
  , m_braceCount(0)
  , m_bracesMatched(true)
{}

void Parser::deleteNodes()
{
  qDeleteAll(m_nodes->begin(), m_nodes->end());
  m_nodes->clear();
}

Parser::~Parser()
{
  deleteNodes();
  delete m_nodes;
}

void Parser::checkBraceCount(const QString& text, ParserState* state)
{
  for (int p = 0; p < text.length(); p++) {
    QChar c = text.at(p);

    if (c == '{') {
      m_braceCount++;

    } else if (c == '}') {
      m_braceCount--;
    }
  }

  if (m_braceCount == 0) {
    m_bracesMatched = true;

  } else {
    // reset braceCount to use in brace detection.
    m_braceCount = 0;
  }

  if (!m_bracesMatched) {
    if (m_braceCount > 0) {
      state->unsetError(ParserState::NoError);
      state->setError(ParserState::MismatchedBraceCount);
      state->setError(ParserState::MissingEndBrace);

      if (!state->errors().testFlag(ParserState::FatalError)) {
        state->setError(ParserState::NonFatalError);
      }

    } else if (m_braceCount < 0) {
      state->unsetError(ParserState::NoError);
      state->setError(ParserState::MismatchedBraceCount);
      state->setError(ParserState::MissingStartBrace);
    }
  }
}

QTextCursor StylesheetEditor::Parser::getNode(int position)
{
  QTextCursor c = QTextCursor(m_editor->document());
  c.movePosition(QTextCursor::Start);
  c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
  return c;
}

ParserState* Parser::parse(const QString& text, int pos)
{
  ParserState* state =  new ParserState(this);
  m_braceCount = 0;
  QString block;
  //  QChar c;
  Node* lastnode = nullptr, *propertynode = nullptr;

  if (text.trimmed().length() == 0) {
    state->unsetError(ParserState::NoError);
    state->setError(ParserState::BlankText);
    return state;
  }

  checkBraceCount(text, state);

  while (true) {
    block = findNext(text, pos, state);

    if (m_datastore->containsWidget(block)) {
      WidgetNode* widgetnode = new WidgetNode(block, getNode(pos - block.length()), this);
      m_nodes->append(widgetnode);
      lastnode = widgetnode;

      while (pos < text.length()) {
        block = findNext(text, pos, state);

        if (block == ":") { // pseudo state
          if (m_braceCount == 0) {
            Node* marker = new PseudoStateMarkerNode(getNode(pos - block.length()), this);
            setNodeLinks(lastnode, marker);
            lastnode = marker;

          } else if (m_braceCount == 1) {  //    if (c.isLetter()) {
            Node* marker = new PropertyMarkerNode(getNode(pos - block.length()), this);
            setNodeLinks(lastnode, marker);
            lastnode = marker;

          } else {
            // TODO missing end brace
          }

          continue;

        } else if (m_braceCount == 1) {
          PropertyNode* property = qobject_cast<PropertyNode*>(propertynode);

          if (property) {
            QStringList values;
            QList<bool> checks;
            QList<int> offsets;
            bool check = false;
            QString propertyName = property->value();
            int start = -1;

            while (true) {
              if (block.isEmpty() || block == ";" || block == "}") {
                break;
              }

              if (start < 0) {
                start = pos - block.length();
              }

              values.append(block);
              check = m_datastore->isValidPropertyValue(propertyName, block);
              checks.append(check);
              // +1 to account for the
              offsets.append(pos - start - block.length());
              block = findNext(text, pos, state);
              continue;
            }

            ValueNode* valuenode = new ValueNode(values, checks, offsets, getNode(start), this);
            setNodeLinks(lastnode, valuenode);
            lastnode = valuenode;
            propertynode = nullptr;

            if (block == ";") { // value end
              Node* endvalues = new SemiColonNode(getNode(pos - block.length()), this);
              setNodeLinks(lastnode, endvalues);  //    if (c.isLetter()) {
              lastnode = endvalues;
              continue;

            } else if (block == "}") { // end brace
              m_braceCount--;
              Node* brace = new EndBraceNode(getNode(pos - block.length()), this);
              setNodeLinks(lastnode, brace);
              lastnode = brace;
              continue;

            }

            continue;

          } else {
            Node* property = new PropertyNode(block, getNode(pos - block.length()), this);
            setNodeLinks(lastnode, property);
            lastnode = property;
            propertynode = property;
            continue;
          }

        } else if (block == "::") { // sub control
          Node* marker = new SubControlMarkerNode(getNode(pos - block.length()), this);
          setNodeLinks(lastnode, marker);
          lastnode = marker;
          continue;

        } else if (block == "{") { // start brace
          m_braceCount++;
          Node* brace = new StartBraceNode(getNode(pos - block.length()), this);
          setNodeLinks(lastnode, brace);
          lastnode = brace;
          continue;

        } else if (block == "}") { // end brace
          m_braceCount--;
          Node* brace = new EndBraceNode(getNode(pos - block.length()), this);
          setNodeLinks(lastnode, brace);
          lastnode = brace;
          continue;

        } else if (block == ";") { // value end
          Node* endvalues = new SemiColonNode(getNode(pos - block.length()), this);
          setNodeLinks(lastnode, endvalues);
          lastnode = endvalues;
          continue;

        } else {
          // TODO non standard block error.
        }

        if (!block.isEmpty()) {
          PseudoStateMarkerNode* pseudostatemarker = qobject_cast<PseudoStateMarkerNode*>(lastnode);

          if (pseudostatemarker) {
            if (m_datastore->containsPseudoState(block)) {
              Node* pseudostate = new PseudoStateNode(block, getNode(pos - block.length()), this);
              setNodeLinks(lastnode, pseudostate);
              lastnode = pseudostate;

            } else {
              Node* badblock = new BadBlockNode(block, getNode(pos - block.length()), this);
              setNodeLinks(lastnode, badblock);
              lastnode = badblock;
            }

            continue;
          }

          SubControlMarkerNode* subcontrolmarker = qobject_cast<SubControlMarkerNode*>(lastnode);

          if (subcontrolmarker) {
            if (m_datastore->containsSubControl(block)) {
              Node* subcontrol = new SubControlNode(block, getNode(pos - block.length()), this);
              setNodeLinks(lastnode, subcontrol);
              lastnode = subcontrol;

            } else {
              Node* badblock = new BadBlockNode(block, getNode(pos - block.length()), this);
              setNodeLinks(lastnode, badblock);
              lastnode = badblock;
            }

            continue;
          }
        }
      }

      skipBlanks(text, pos);
    }

    if (pos >= text.length()) {
      break;
    }
  }

  return state;
}

/*
  Finds the next character block.
  This block can end with a space, a colon, a semicolon, a start curly brace
  or an end curly brace. Multipe copies of these end characters are allowed.
  Some of blocks these are good, but some are errors.
  for example a double colon is good but a double curly start brace is not good.
*/
QString Parser::findNext(const QString& text, int& pos, ParserState* state)
{
  QString block;
  QChar c;
  skipBlanks(text, pos);
  c = text.at(pos);

  while (true) {
    if (c.isNull() || pos >= text.length()) {
      return block;
    }

    if (c.isLetterOrNumber()) {
      if (!block.isEmpty()) {
        QChar b = block.back();

        if (b == '{' || b == '}' || b == ';' || b == ':') {
          return block;
        }
      }

      block += c;
      pos++;

    } else if (c.isSpace() && !block.isEmpty()) {
      return block;

    } else if (c == '{' || c == '}' || c == ';' || c == ':') {
      if (!block.isEmpty()) {
        if (block.back().isLetterOrNumber()) {
          // a possibly correct name/number string
          return block;
        }
      }

      if (block.length() == 0 || block.back() == c) {
        block += c;
        pos++;

      } else {
        return block;
      }
    }

    if (pos < text.length()) {
      c = text.at(pos);
    }
  }
}

void Parser::setNodeLinks(Node* first, Node* second)
{
  first->next = second;
  second->previous = first;
}

NodeList* Parser::nodes()
{
  return m_nodes;
}

void Parser::skipBlanks(const QString& text, int& pos)
{
  QChar c;

  for (; pos < text.length(); pos++) {
    c = text.at(pos);

    if (c.isSpace()/* || c == '\n' || c == '\r' || c == '\t'*/) {
      continue;

    } else {
      break;
    }
  }
}

} // end of StylesheetParser
