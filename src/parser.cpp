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

namespace StylesheetParser {

Parser::Parser(QObject* parent)
  : QObject(parent)
  , m_nodes(new QList<Node*>())
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

ParserState* Parser::parse(const QString& text)
{
  ParserState::Errors errors = ParserState::NoError;
  int braceCount = 0;
  int pos = 0;
  Node* node = nullptr;

  // mark all newline chars for use later.
  // capture newline chars as skipBlanks() skips over thwm.
  for (int p = pos; p < text.length(); p++) {
    QChar c = text.at(p);

    if (c == '\n') {
      node = new NewlineNode(p, this);
      m_nodes->append(node);
    }
  }

  for (; pos < text.length(); pos++) {
    QChar c = text.at(pos);

    if (c == '\n') {
      // capture newline char before skipBlanks() skips over it.
      node = new NewlineNode(pos, this);
      m_nodes->append(node);
    }

    skipBlanks(text, pos);
    c = text.at(pos);

    if (c.isLetter()) {
      if (braceCount == 0) {
        // must be either a class name OR a style without OR an error
        node = findName(text, pos, braceCount);
        m_nodes->append(node);
      }

    } else if (c == '{') {
      braceCount++;
      // must be either a class name OR a style without OR an error
      node = new StartBraceNode(pos, this);
      m_nodes->append(node);

    } else if (c == "}") {
      braceCount--;
      node = new EndBraceNode(pos, this);
      m_nodes->append(node);

    } else if (c == ':') {
      if (braceCount == 0) {
        if (pos < text.length() - 1) {
          if (text.at(pos + 1) == ':') {
            // :: sub_control
            node = new SubControlMarkerNode(pos, this);
            m_nodes->append(node);
            skipBlanks(text, pos);
            pos += 2;
            node = findSubControl(text, pos);
            m_nodes->append(node);

          } else {
            // : pseudo state
            node = new PseudoStateMarkerNode(pos, this);
            m_nodes->append(node);
            skipBlanks(text, pos);
            pos++;
            node = findPseudoState(text, pos);
            m_nodes->append(node);
          }
        }
      }

    }
  }

  ParserState* state =  new ParserState(errors, this);

  if (braceCount > 0) {
    state->unsetError(ParserState::NoError);
    state->setError(ParserState::MismatchedBraceCount);
    state->setError(ParserState::MissingEndBrace);

    if (!errors.testFlag(ParserState::FatalError)) {
      state->setError(ParserState::NonFatalError);
    }

  } else if (braceCount < 0) {
    state->unsetError(ParserState::NoError);
    state->setError(ParserState::MismatchedBraceCount);
    state->setError(ParserState::MissingStartBrace);

    if (!errors.testFlag(ParserState::FatalError)) {
      state->setError(ParserState::NonFatalError);
    }

  }

  return state;

}

Node* Parser::findName(const QString& text, int& pos, int& braceCount)
{
  QString name;
  Node* node = nullptr;
  int start = pos;

  for (; pos < text.length(); pos++) {
    QChar c = text.at(pos);

    if (c.isSpace() || c == '{' || c == ':') {
      if (braceCount == 0) {
        if (WidgetNode::contains(name)) {
          node = new WidgetNode(name, start, this);
          pos--; // step back from the last character
          break;

        } else {
          node = new NameNode(name, start, this);
          pos--; // step back from the last character
        }

        // TODO handle property name.

        break;

      } else {
        // TODO neither widget name or pseudo-state
      }
    } else if (c.isLetter()) {
      name += c;
      continue;

    }
  }

  if (!node && name.length() > 0) {
    node = new NameNode(name, start, this);
  }

  return node;
}

Node* Parser::findSubControl(const QString& text, int& pos)
{
  QString name;
  Node* node = nullptr;
  int start = pos;

  for (; pos < text.length(); pos++) {
    QChar c = text.at(pos);

    if (c.isLetter()) {
      name += c;
      continue;

    } else if (c.isSpace() || c == '{') {
      if (SubControlNode::contains(name)) {
        node = new SubControlNode(name, start, this);
        pos--; // step back from the last characters
        break;
      }
    }
  }

  return node;
}

Node* Parser::findPseudoState(const QString& text, int& pos)
{
  QString name;
  Node* node = nullptr;
  int start = pos;

  for (; pos < text.length(); pos++) {
    QChar c = text.at(pos);

    if (c.isLetter()) {
      name += c;
      continue;

    } else if (c.isSpace() || c == '{') {
      if (PseudoStateNode::contains(name)) {
        node = new PseudoStateNode(name, start, this);
        pos--; // step back from the last character
        break;
      }
    }
  }

  return node;
}


QList<Node*>* Parser::nodes()
{
  return m_nodes;
}

void Parser::skipBlanks(const QString& text, int& pos)
{
  for (; pos < text.length(); pos++) {
    QChar c = text.at(pos);

    if (c.isSpace() || c == '\n' || c == '\r' || c == '\t') {
      continue;

    } else {
      break;
    }
  }
}


} // end of StylesheetParser
