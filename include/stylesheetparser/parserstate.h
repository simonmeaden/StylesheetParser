/*
   Copyright 2020 Simon Meaden

   Permission is hereby granted, free of charge, to any person obtaining a copy of this
   software and associated documentation files (the "Software"), to deal in the Software
   without restriction, including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software, and to permit
                                                                         persons to whom the Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all copies or
   substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
    PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef PARSERERROR_H
#define PARSERERROR_H

#include <QObject>
//#include <QException>

namespace StylesheetParser {

class ParserState : public QObject
{
public:
   enum Error
   {
      NoError = 0x1,
      FatalError = 0x2,
      NonFatalError = 0x4,
      BlankText = 0x10,
      WronglyPositionedSpace = 0x20,
      IncorrectlyTerminatedName = 0x40,
      MismatchedBraceCount = 0x80,
      MissingEndBrace = 0x100,
      MissingStartBrace = 0x200,
   };
   Q_DECLARE_FLAGS(Errors, Error)
   Q_FLAG(Errors)
   enum State
   {
      ColonFound = 0x1,
      SemicolonFound = 0x2,
      TerminatingSpaceFound = 0x4,
      TerminatingBraceFound = 0x8,
      StartingBraceFound = 0x10,
      TextFinished = 0x20,
   };

   ParserState(Errors errors, QObject* parent);
   ParserState(Errors errors, int pos, QObject* parent);
   ParserState(Errors errors, int pos, const QString& value, QObject* parent);

   QString value() const;
   void setValue(const QString& value);

   Errors errors() const;
   void setError(Error error);
   void unsetError(Error error);

   int pos() const;
   void setPos(int pos);

private:
   Errors m_errors;
   int m_pos;
   QString m_value;
};

} // end of StylesheetParser

Q_DECLARE_METATYPE(StylesheetParser::ParserState::Error)
Q_DECLARE_OPERATORS_FOR_FLAGS(StylesheetParser::ParserState::Errors)

#endif // PARSERERROR_H
