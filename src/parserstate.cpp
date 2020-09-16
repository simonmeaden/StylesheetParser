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
#include "stylesheetparser/parserstate.h"

namespace StylesheetEditor {

ParserState::ParserState(QObject *parent)
  : QObject(parent)
  , m_pos(-1)
  , m_value(QString())
{

}

ParserState::ParserState(Errors errors, QObject* parent = nullptr)
   : QObject(parent)
   , m_errors(errors)
   , m_pos(-1)
   , m_value(QString())
{

}

ParserState::ParserState(Errors errors, int pos, QObject* parent)
   : QObject(parent)
   , m_errors(errors)
   , m_pos(pos)
   , m_value(QString())
{

}

ParserState::ParserState(Errors errors, int pos, const QString& value, QObject* parent)
   : QObject(parent)
   , m_errors(errors)
   , m_pos(pos)
   , m_value(value)
{

}

QString ParserState::value() const
{
   return m_value;
}

void ParserState::setValue(const QString& value)
{
   m_value = value;
}

ParserState::Errors ParserState::errors() const
{
   return m_errors;
}

void ParserState::setError(ParserState::Error error) {
  m_errors |= error;
}

void ParserState::unsetError(ParserState::Error error) {
  m_errors = m_errors & (~error);
}

bool ParserState::hasError(ParserState::Error error)
{
  return m_errors.testFlag(error);
}

int ParserState::pos() const
{
  return m_pos;
}

void ParserState::setPos(int pos)
{
  m_pos = pos;
}

} // end of StylesheetParser
