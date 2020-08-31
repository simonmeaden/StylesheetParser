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

void Parser::findName(const QString &text, int pos)
{

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

   for (auto c : text) {
      if (c == '{') {
         braceCount++;

      } else if (c == "}") {
         braceCount--;
      }

      if (c.isLetter()) {
         if (braceCount == 0) {
           // must be either a class name OR a style without OR an error
           pos = findName(text, pos);
         }
      } else {
        // error ???
      }
   }

   if (braceCount != 0) {
      errors |= ParserState::MismatchedBraceCount;

   }

   if (braceCount > 0) {
      errors |= ParserState::MissingEndBrace;

   } else if (braceCount < 0) {
      errors |= ParserState::MissingStartBrace;

   }

   if (errors == ParserState::NoError) {
      return new ParserState(ParserState::NoError, this);

   } else {
      // there are errors so remove NoError flag.
      return new ParserState(errors | ParserState::NoError, this);
   }

   return new ParserState(errors, this);

   //   int pos = 0, start;
   //   QString  value;
   //   deleteNodes();

   //   ParserState* state;
   //   bool hasStartBrace = false;

   //   do {

   //      pos = skipBlanks(text, pos);

   //      state = parseNameBraceOrNameValue(text, pos, hasStartBrace);
   //      pos = state->pos();

   //      if (!state->errors().testFlag(ParserState::NoError) |
   //          state->errors().testFlag(ParserState::TextFinished)) {
   //        return state;
   //      }

   //      do {
   //         pos = skipBlanks(text, pos);

   //         state = parseNameValue(text, pos, hasStartBrace);
   //         pos = state->pos();

   //         if (!state->errors().testFlag(ParserState::NoError) |
   //               state->errors().testFlag(ParserState::TextFinished)) {
   //            return state;
   //         }

   //      } while (pos < text.length());
   //   } while (pos < text.length());

   //   return new ParserState(ParserState::NoError, this);
}

QList<Node*>* Parser::nodes()
{
   return m_nodes;
}

int Parser::skipBlanks(const QString& text, int pos)
{
   int p = pos;

   for (; p < text.length(); p++) {
      QChar c = text.at(p);

      if (c.isSpace() || c == '\n' || c == '\r' || c == '\t') {
         continue;

      } else {
         break;
      }
   }

   return p;
}

//ParserState* Parser::parseNameBraceOrNameValue(const QString& text, int pos, bool& hasStartBrace)
//{
//   ParserState* state;
//   ParserState::Errors errors;

//   state = getName(text, pos);
//   errors = state->errors();
//   QString value = state->value();
//   int start = state->pos();
//   pos = start + value.length();

//   if (!errors.testFlag(ParserState::NoError)) {
//      return state;

//   } else {
//      if (errors.testFlag(ParserState::TextFinished)) {
//         m_nodes->append(new NameNode(value, start, this));
//         return state;

//      } else if (errors.testFlag(ParserState::StartingBraceFound)) {
//         m_nodes->append(new NameNode(value, start, this));
//         pos++;
//         m_nodes->append(new StartBraceNode(pos, this));
//         pos++;
//         hasStartBrace = true;
//         state->setPos(pos);

//      } else if (errors.testFlag(ParserState::ColonFound)) {
//         m_nodes->append(new NameNode(value, start, this));
//         pos++;
//         m_nodes->append(new ColonNode(pos, this));
//         pos++;
//         state->setPos(pos);

//         pos = skipBlanks(text, pos);

//         state = parseValue(text, pos, hasStartBrace);
//         int start = state->pos();
//         pos = start + value.length();
//         state->setPos(pos);

//         if (!state->errors().testFlag(ParserState::NoError)) {
//            return state;
//         }

//      } else {
//         return state;
//      }

//   }

//   return state;
//}

//ParserState* Parser::parseNameValue(const QString& text, int pos, bool& hasStartBrace)
//{
//   ParserState* state;
//   ParserState::Errors errors;

//   state = getName(text, pos);
//   errors = state->errors();
//   QString value = state->value();
//   int start = state->pos();
//   pos = start + value.length();

//   if (!errors.testFlag(ParserState::NoError)) {
//      return state;

//   } else {
//      if (errors.testFlag(ParserState::TextFinished)) {
//         m_nodes->append(new NameNode(value, start, this));
//         return state;

//      } else if (errors.testFlag(ParserState::ColonFound)) {
//         m_nodes->append(new NameNode(value, start, this));
//         pos++;
//         m_nodes->append(new ColonNode(pos, this));
//         pos++;
//         state->setPos(pos);

//         pos = skipBlanks(text, pos);

//         state = parseValue(text, pos, hasStartBrace);
//         int start = state->pos();
//         pos = start + value.length();
//         state->setPos(pos);

//         if (!state->errors().testFlag(ParserState::NoError)) {
//            return state;
//         }

//      } else {
//         return state;
//      }

//   }

//   return state;
//}

//ParserState* Parser::getName(const QString& text, int pos)
//{
//   QString name;
//   int start = pos;

//   while (pos < text.length()) {
//      QChar c = text.at(pos);

//      if (pos == text.length()) {
//         return new ParserState(ParserState::TextFinished, pos, this);

//      } else if (c.isSpace()) {
//         if (name.isEmpty()) {
//            return new ParserState(ParserState::WronglyPositionedSpace, pos, this);

//         } else {
//            pos = skipBlanks(text, pos);

//            if (pos >= text.length()) {
//               return new ParserState(
//                         ParserState::NoError | ParserState::TextFinished, start, name, this);

//            } else if (text.at(pos) == ":") {
//               return new ParserState(
//                         ParserState::NoError | ParserState::ColonFound, start, name, this);

//            } else if (text.at(pos) == "{") {
//               return new ParserState(
//                         ParserState::NoError | ParserState::StartingBraceFound, start, name, this);

//            } else {
//               return new ParserState(
//                         ParserState::IncorrectlyTerminatedName, pos, name, this);
//            }
//         }

//      } else if (c == ':') {
//         return new ParserState(
//                   ParserState::NoError | ParserState::ColonFound, start, name, this);

//      } else if (c.isLetter()) {
//         name += c;
//      }

//      pos++;
//   }

//   return new ParserState(ParserState::BlankText, text.length(), this);
//}

//ParserState* Parser::parseValue(const QString& text, int pos, bool& hasStartBrace)
//{
//   ParserState* state;
//   ParserState::Errors errors;

//   state = getValue(text, pos);
//   errors = state->errors();
//   QString value = state->value();
//   int start = state->pos();
//   pos = start + value.length();

//   if (!errors.testFlag(ParserState::NoError)) {
//      return state;

//   } else {
//      if (errors.testFlag(ParserState::TextFinished)) {
//         m_nodes->append(new ValueNode(value, start, this));
//         return state;

//      } else if (errors.testFlag(ParserState::SemicolonFound)) {
//         m_nodes->append(new ValueNode(value, start, this));
//         pos++;
//         m_nodes->append(new SemiColonNode(pos, this));
//         pos++;

//      } else if (errors.testFlag(ParserState::TerminatingSpaceFound)) {
//         m_nodes->append(new ValueNode(value, start, this));

//      }  else if (errors.testFlag(ParserState::TerminatingBraceFound)) {
//         m_nodes->append(new ValueNode(value, start, this));
//         pos++;

//         if (hasStartBrace) {
//            m_nodes->append(new EndBraceNode(pos, this));
//            hasStartBrace = false;

//         } else {
//            // TODO handle error with no start brace.
//         }

//      } else {
//         return state;
//      }
//   }

//   return state;
//}

//ParserState* Parser::getValue(const QString& text, int pos)
//{
//   QString value;
//   int start = pos;

//   while (true) {
//      QChar c = text.at(pos);

//      if (c.isSpace()) {
//         if (value.isEmpty()) {
//            return new ParserState(ParserState::WronglyPositionedSpace, pos, this);

//         } else {
//            pos = skipBlanks(text, pos);

//            if (pos == text.length()) {
//               return new ParserState(
//                         ParserState::NoError | ParserState::TextFinished, start, value, this);

//            } else if (text.at(pos) == ":") {
//               return new ParserState(
//                         ParserState::NoError | ParserState::ColonFound, start, value, this);

//            } else {
//               return new ParserState(
//                         ParserState::IncorrectlyTerminatedName, pos, value, this);
//            }
//         }

//      } else if (c == ';') {
//         return new ParserState(
//                   ParserState::NoError | ParserState::SemicolonFound, start, value, this);

//      } else if (c.isLetter()) {
//         value += c;
//      }

//      pos++;
//   }

//   return new ParserState(ParserState::BlankText, text.length(), this);
//}

} // end of StylesheetParser
