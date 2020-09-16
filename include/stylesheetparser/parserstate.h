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

namespace StylesheetEditor {

class ParserState : public QObject
{
public:
  enum Error
  {
    NoError                             = 0x1, //!< No parsing error
    FatalError                          = 0x2, //!< Fatal parsing error
    NonFatalError                       = 0x4, //!< Non fatal parsing error
    BlankText                           = 0x10, //!< No text so far
    WronglyPositionedSpace              = 0x20, //!< A space in the wrong place
    IncorrectlyTerminatedName           = 0x40, //!<
    MismatchedBraceCount                = 0x80, //!< Either a extra start or end curly brace
    MissingEndBrace                     = 0x100, //!< A missing  end curly brace
    MissingStartBrace                   = 0x200, //!< A missing start brace
    TooManyCharactersInBlock            = 0x400, //!<
    PseudoStateMarkerNotFollowingWidget = 0x800, //!< A pseudo state must folow a valid widget name
    SubControlMarkerNotFollowingWidget  = 0x1000, //!< A sub control must folow a valid widget name
    NotAValidPropertyName               = 0x2000, //!< Not a valid property name.
    IncorrectPropertyEnder              = 0x4000, //!< A property/value list must end in ';', '}' or the end of text.
  };
  Q_DECLARE_FLAGS(Errors, Error)
  Q_FLAG(Errors)
  //   enum State
  //   {
  //      ColonFound = 0x1,
  //      SemicolonFound = 0x2,
  //      TerminatingSpaceFound = 0x4,
  //      TerminatingBraceFound = 0x8,
  //      StartingBraceFound = 0x10,
  //      TextFinished = 0x20,
  //   };

  ParserState(QObject* parent);
  ParserState(Errors errors, QObject* parent);
  ParserState(Errors errors, int pos, QObject* parent);
  ParserState(Errors errors, int pos, const QString& value, QObject* parent);

  QString value() const;
  void setValue(const QString& value);

  Errors errors() const;
  void setError(Error error);
  void unsetError(Error error);
  bool hasError(Error error);

  int pos() const;
  void setPos(int pos);

private:
  Errors m_errors;
  int m_pos;
  QString m_value;
};

} // end of StylesheetEditor

Q_DECLARE_METATYPE(StylesheetEditor::ParserState::Error)
Q_DECLARE_OPERATORS_FOR_FLAGS(StylesheetEditor::ParserState::Errors)

#endif // PARSERERROR_H
