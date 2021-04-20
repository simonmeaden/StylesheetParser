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

// const QStringList PropertyStatus::names = QStringList() << "GoodName"
//                                                        << "BadName"
//                                                        << "FuzzyName"
//                                                        << "FuzzyColorValue"
//                                                        << "GoodValueName"
//                                                        << "BadValueName"
//                                                        << "FuzzyValueName"
//                                                        << "GoodValue"
//                                                        << "BadValue"
//                                                        << "BadValueCount"
//                                                        << "BadNumericalValue"
//                                                        << "BadLengthUnit"
//                                                        << "BadFontUnit"
//                                                        << "BadColorValue"
//                                                        << "BadUrlValue"
//                                                        << "RepeatValueName"
//                                                        << "FuzzyColorValue"
//                                                        << "OpenParentheses"
//                                                        << "CloseParentheses";

// QDebug
// operator<<(QDebug debug, const PropertyStatus& status)
//{
//  QDebugStateSaver saver(debug);
//  debug.nospace() << "State : " << status.toString()
//                  << " name : " << status.m_name;
//  debug.nospace() << "  offset : " << status.m_offset
//                  << " length : " << status.length();
//  return debug;
//}

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

int
PropertyStatus::maxCount() const
{
  return m_maxCount;
}

void
PropertyStatus::setMaxCount(int maxCount)
{
  m_maxCount = maxCount;
}

int
PropertyStatus::minCount() const
{
  return m_minCount;
}

void
PropertyStatus::setMinCount(int minCount)
{
  m_minCount = minCount;
}
