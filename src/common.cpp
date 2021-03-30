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
#include "common.h"
#include "node.h"

bool
operator==(NodeSection& left, NodeSection& right)
{
  return (left.type == right.type && left.position == right.position &&
          left.node == right.node && left.propertyIndex == right.propertyIndex);
}

bool
operator!=(NodeSection& left, NodeSection& right)
{
  return !(left == right);
}

const QStringList PropertyStatus::names =
  QStringList() << "IrrelevantValue" << // Not relevant for this check.
  "GoodPropertyValue" <<                //! Good value for this property name
  "BadPropertyValue" <<                 //! Bad general value
  "FuzzyColorValue"
  //                << "EmptyGradientValueName" << //! Missing value
  "FuzzyGradientName"
                << "GoodGradientName" << //! Bad gradient name
  //  "BadGradientName" <<                   //! Bad gradient name
  "BadGradientValue" <<                  //! Bad gradient value.
  "BadGradientNumericalValue" <<         //! Bad gradient number.
  "BadGradientColorValue" <<             //! Bad gradient color.
  "BadGradientNumericalAndColorValue" << //! Bad gradient number and color.
  "BadGradientValueCount" <<             //! Bad gradient value count.
  "BadGradientValueName" <<              //! Bad gradient value.
  "RepeatedGradientValue";               //! Bad gradient value.

PropertyStatus::PropertyStatus(PropertyValueState s,
                               const QString& n,
                               int o,
                               int l)
  : state(s)
  , offset(o)
  , length(l)
  , name(n)
{
  qDebug();
}

QDebug
operator<<(QDebug debug, const PropertyStatus& status)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "State : " << status.toString()
                  << " name : " << status.name;
  debug.nospace() << "  offset : " << status.offset
                  << " length : " << status.length;
  return debug;
}

static const QStringList NodeTypeNames = QStringList()
                                         << "NoType" <<
                                         //  "NodeType"<<
                                         "WidgetType"
                                         << "WidgetsType" <<
                                         //  "FuzzyWidgetType"<<
                                         "PseudoStateType" <<
                                         //  "FuzzyPseudoStateType"<<
                                         "PseudoStateMarkerType"
                                         << "SubControlType" <<
                                         //  "FuzzySubControlType"<<
                                         "SubControlMarkerType"
                                         << "IdSelectorType"
                                         << "IdSelectorMarkerType"
                                         << "ColonType"
                                         << "StartBraceType"
                                         << "EndBraceType"
                                         << "NewlineType"
                                         << "PropertyType" <<
                                         //  "FuzzyPropertyType"<<
                                         "PropertyMarkerType"
                                         << "PropertyValueType" <<
                                         //  "FuzzyPropertyValueType"<<
                                         "BadNodeType"
                                         << "CommentType"
                                         << "CommentStartMarkerType"
                                         << "CommentEndMarkerType";
QDebug
operator<<(QDebug debug, const NodeType& type)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "Type : " << NodeTypeNames.at(int(type));
  return debug;
}
