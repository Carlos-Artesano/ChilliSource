//
//  LayoutDesc.cpp
//  Chilli Source
//  Created by Ian Copland on 21/11/2014.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2014 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#include <ChilliSource/UI/Layout/LayoutDesc.h>

#include <ChilliSource/Core/String/StringParser.h>
#include <ChilliSource/Core/Json/JsonUtils.h>

#include <json/json.h>

namespace ChilliSource
{
    namespace UI
    {
        namespace
        {
            const char k_layoutTypeKey[] = "Type";
            const char k_relativeMarginsKey[] = "RelMargins";
            const char k_absoluteMarginsKey[] = "AbsMargins";
            
            //--------------------------------------------------------------
            /// Parses the given cell order string. If the string is not
            /// a valid cell order the app is considered to be in an
            /// irrecoverable state and will terminate.
            ///
            /// @author Ian Copland
            ///
            /// @param The cell order in string form.
            ///
            /// @return The parsed cell order.
            //--------------------------------------------------------------
            GridLayout::CellOrder ParseCellOrder(const std::string& in_cellOrderString)
            {
                const char k_columnMajor[] = "colmajor";
                const char k_rowMajor[] = "rowmajor";
                
                std::string cellOrderString = in_cellOrderString;
                Core::StringUtils::ToLowerCase(cellOrderString);
                
                if(cellOrderString == k_columnMajor)
                {
                    return GridLayout::CellOrder::k_colMajor;
                }
                else if(cellOrderString == k_rowMajor)
                {
                    return GridLayout::CellOrder::k_rowMajor;
                }
                
                CS_LOG_FATAL("Unknown GridLayout cell order: " + in_cellOrderString);
                return GridLayout::CellOrder::k_colMajor;
            }
        }

        //--------------------------------------------------------------
        //--------------------------------------------------------------
        LayoutDesc::LayoutDesc(const Json::Value& in_json)
        {
            const auto& typeJson = in_json.get(k_layoutTypeKey, Json::nullValue);
            CS_ASSERT(typeJson != Json::nullValue, "'" + std::string(k_layoutTypeKey) + "' must be specified in a Layout Description.");
            
            m_type = ParseLayoutType(typeJson.asString());
            switch (m_type)
            {
                case LayoutType::k_grid:
                    InitAsGrid(in_json);
                    break;
                case LayoutType::k_hList:
                case LayoutType::k_vList:
                    InitAsList(in_json);
                    break;
                default:
                    CS_LOG_FATAL("Invalid layout type.");
                    break;
            }
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        LayoutType LayoutDesc::GetType() const
        {
            return m_type;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        Core::Vector4 LayoutDesc::GetRelativeMargins() const
        {
            return m_relativeMargins;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        Core::Vector4 LayoutDesc::GetAbsoluteMargins() const
        {
            return m_absoluteMargins;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        u32 LayoutDesc::GetNumRows() const
        {
            return m_numRows;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        u32 LayoutDesc::GetNumCols() const
        {
            return m_numCols;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        f32 LayoutDesc::GetRelativeHSpacing() const
        {
            return m_relativeHSpacing;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        f32 LayoutDesc::GetAbsoluteHSpacing() const
        {
            return m_absoluteHSpacing;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        f32 LayoutDesc::GetRelativeVSpacing() const
        {
            return m_relativeVSpacing;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        f32 LayoutDesc::GetAbsoluteVSpacing() const
        {
            return m_absoluteVSpacing;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        GridLayout::CellOrder LayoutDesc::GetCellOrder() const
        {
            return m_cellOrder;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        f32 LayoutDesc::GetRelativeSpacing() const
        {
            return m_relativeSpacing;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        f32 LayoutDesc::GetAbsoluteSpacing() const
        {
            return m_absoluteSpacing;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        u32 LayoutDesc::GetNumCells() const
        {
            return m_numCells;
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        void LayoutDesc::InitAsGrid(const Json::Value& in_json)
        {
            const char k_numRowsKey[] = "NumRows";
            const char k_numColumnsKey[] = "NumCols";
            const char k_relativeHSpacingKey[] = "RelHSpacing";
            const char k_absoluteHSpacingKey[] = "AbsHSpacing";
            const char k_relativeVSpacingKey[] = "RelVSpacing";
            const char k_absoluteVSpacingKey[] = "AbsVSpacing";
            const char k_cellOrderKey[] = "CellOrder";
            
            for(auto it = in_json.begin(); it != in_json.end(); ++it)
            {
                CS_ASSERT((*it).isString() == true, "All properties in a Layout Description must be a string: " + std::string(it.memberName()));
                
                std::string key = it.memberName();
                std::string value = (*it).asString();
                
                if (key == k_relativeMarginsKey)
                {
                    m_relativeMargins = Core::ParseVector4(value);
                }
                else if (key == k_absoluteMarginsKey)
                {
                    m_absoluteMargins = Core::ParseVector4(value);
                }
                else if (key == k_numRowsKey)
                {
                    m_numRows = Core::ParseU32(value);
                }
                else if (key == k_numColumnsKey)
                {
                    m_numCols = Core::ParseU32(value);
                }
                else if (key == k_relativeHSpacingKey)
                {
                    m_relativeHSpacing = Core::ParseF32(value);
                }
                else if (key == k_absoluteHSpacingKey)
                {
                    m_absoluteHSpacing = Core::ParseF32(value);
                }
                else if (key == k_relativeVSpacingKey)
                {
                    m_relativeVSpacing = Core::ParseF32(value);
                }
                else if (key == k_absoluteVSpacingKey)
                {
                    m_absoluteVSpacing = Core::ParseF32(value);
                }
                else if (key == k_cellOrderKey)
                {
                    m_cellOrder = ParseCellOrder(value);
                }
                else if (key == k_layoutTypeKey)
                {
                    //ignore
                }
                else
                {
                    CS_LOG_FATAL("Invalid property found in a Grid layout description: " + key);
                }
            }
        }
        //--------------------------------------------------------------
        //--------------------------------------------------------------
        void LayoutDesc::InitAsList(const Json::Value& in_json)
        {
            const char k_numCellsKey[] = "NumCells";
            const char k_relativeSpacingKey[] = "RelSpacing";
            const char k_absoluteSpacingKey[] = "AbsSpacing";
            
            for(auto it = in_json.begin(); it != in_json.end(); ++it)
            {
                CS_ASSERT((*it).isString() == true, "All properties in a Layout Description must be a string: " + std::string(it.memberName()));
                
                std::string key = it.memberName();
                std::string value = (*it).asString();
                
                if (key == k_relativeMarginsKey)
                {
                    m_relativeMargins = Core::ParseVector4(value);
                }
                else if (key == k_absoluteMarginsKey)
                {
                    m_absoluteMargins = Core::ParseVector4(value);
                }
                else if (key == k_numCellsKey)
                {
                    m_numCells = Core::ParseU32(value);
                }
                else if (key == k_relativeSpacingKey)
                {
                    m_relativeSpacing = Core::ParseF32(value);
                }
                else if (key == k_absoluteSpacingKey)
                {
                    m_absoluteSpacing = Core::ParseF32(value);
                }
                else if (key == k_layoutTypeKey)
                {
                    //ignore
                }
                else
                {
                    CS_LOG_FATAL("Invalid property found in a List layout description: " + key);
                }
            }
        }
    }
}