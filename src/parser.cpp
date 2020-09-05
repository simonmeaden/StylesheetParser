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
  , m_datastore(new DataStore(this))
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
  bool bracesMatched = false;
  int pos = 0;
  Node* node = nullptr;
  QString name;

  // Mark all newline chars and check for matching brace pairs for use later use.
  // Capture newline chars as skipBlanks() skips over them.
  for (int p = pos; p < text.length(); p++) {
    QChar c = text.at(p);

    if (c == '\n') {
      node = new NewlineNode(p, this);
      m_nodes->append(node);

    } else if (c == '{') {
      braceCount++;

    } else if (c == '}') {
      braceCount--;
    }
  }

  if (braceCount == 0) {
    bracesMatched = true;

  } else {
    // reset braceCount to use in brace detection.
    braceCount = 0;
  }

  for (; pos < text.length(); pos++) {
    QChar c = text.at(pos);

    if (c == '\n') {
      // capture newline char before skipBlanks() skips over it.
      node = new NewlineNode(pos, this);
      m_nodes->append(node);
    }

    skipBlanks(text, pos);

    if (pos < text.length()) {
      c = text.at(pos);

      if (c.isLetter()) {
        if (braceCount == 0) {
          // must be either a class name OR a style without OR an error
          node = findName(text, pos, braceCount);
          NameNode* nNode = qobject_cast<NameNode*>(node);

          if (nNode) {
            name = nNode->value();
          }

          m_nodes->append(node);

        } else if (braceCount == 1) {
          findPropertyAndValues(text, pos);
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

      } else if (c == ";") {
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
  }

  ParserState* state =  new ParserState(errors, this);

  if (!bracesMatched) {
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
        if (m_datastore->containsWidget(name)) {
          node = new WidgetNode(name, start, this);
          pos--; // step back from the last character
          break;

        } else if (m_datastore->containsProperty(name)) {
          pos = start; // step back to start opf name and try again.
          findPropertyAndValues(text, pos);

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
      if (m_datastore->containsWidget(name)) {
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
      if (m_datastore->containsPseudoState(name)) {
        node = new PseudoStateNode(name, start, this);
        pos--; // step back from the last character
        break;
        node = new NameNode(name, start, this);

      }
    }
  }

  return node;
}

void Parser::findPropertyAndValues(const QString& text, int& pos)
{
  QString propertyName;
  PropertyNode* pNode = nullptr;
  int start = pos;

  for (; pos < text.length(); pos++) {
    QChar c = text.at(pos);

    if (c.isSpace() || c == '{' || c == ':') {
      if (m_datastore->containsProperty(propertyName)) {
        pNode = new PropertyNode(propertyName, start, this);
        m_nodes->append(pNode);

        skipBlanks(text, pos);

        if (pos < text.length() && text.at(pos) == ':') {
          PropertyMarkerNode* node = new PropertyMarkerNode(pos, this);
          m_nodes->append(node);
          pos++;
          skipBlanks(text, pos);
        }

        findValues(text, pos, pNode);

        break;

      } else {
        // TODO some kind of error??
      }

      // TODO handle property name.

      break;

    } else if (c.isLetter()) {
      propertyName += c;
      continue;

    }
  }
}

void StylesheetParser::Parser::findValues(const QString& text, int& pos, PropertyNode* pNode)
{
  QStringList values;
  QList<bool> checks;
  QList<int> offsets;
  QString value;
  Node* node = nullptr;
  bool check = false, semicolon = false, endbrace = false;
  int start = pos;
  QString propertyName;

  if (pNode) {
    propertyName = pNode->value();
  }

  for (; pos < text.length(); pos++) {
    QChar c = text.at(pos);

    if (c.isSpace() || c == ';' || c == '}') {
      values.append(value);
      check = m_datastore->isValidPropertyValue(propertyName, value);
      checks.append(check);
      offsets.append(pos - value.length() - start);


      value.clear();
      skipBlanks(text, pos);
      pos--;  // step back from the last character

      if (c.isSpace()) {
        continue;

      } else if (c == ';') {
        // a property value list ends with a semicolon
        semicolon = true;
        break;

      } else if (c == '}') {
        // OR optionally a brace if it is the last property in
        // the property list.
        endbrace = true;
        break;

      }

      continue;

    } else if (c.isLetter() || c.isNumber()) {
      value += c;
      continue;

    }
  }

  if (!values.isEmpty() && !checks.isEmpty() && (values.size() == checks.size())) {
    node = new ValueNode(values, checks, offsets, start, this);
    m_nodes->append(node);

  } else {
    // TODO some form of error.
  }

  if (semicolon) {
    // a property value list ends with a semicolon
    node = new SemiColonNode(pos, this);
    m_nodes->append(node);

  } else if (endbrace) {
    // OR optionally a brace if it is the last property in
    // the property list.
    node = new EndBraceNode(pos, this);
    m_nodes->append(node);
  }

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
