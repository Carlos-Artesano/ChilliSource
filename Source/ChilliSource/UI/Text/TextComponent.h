//
//  TextComponent.h
//  Chilli Source
//  Created by Ian Copland on 05/11/2014.
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

#ifndef _CHILLISOURCE_UI_TEXT_TEXTCOMPONENT_H_
#define _CHILLISOURCE_UI_TEXT_TEXTCOMPONENT_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/Core/Base/Colour.h>
#include <ChilliSource/Core/Container/Property/PropertyMap.h>
#include <ChilliSource/Rendering/Base/CanvasRenderer.h>
#include <ChilliSource/Rendering/Base/HorizontalTextJustification.h>
#include <ChilliSource/Rendering/Base/VerticalTextJustification.h>
#include <ChilliSource/Rendering/Font/Font.h>
#include <ChilliSource/Rendering/Texture/UVs.h>
#include <ChilliSource/UI/Base/Component.h>
#include <ChilliSource/UI/Text/TextIcon.h>

#include <vector>

namespace ChilliSource
{
    namespace UI
    {
        //------------------------------------------------------------------------------
        /// A component which can be added to a widget to provide text rendering
        /// functionality. Text drawable contains the following properties:
        ///
        /// "Font": The description of the font resource that will be used to render
        /// the text. This default to the default font built into chilli source.
        ///
        /// "LocalisedText": The description of the localised text resource which
        /// will be used to generate the text.
        ///
        /// "LocalisedTextId": The Id of the string in the given localised text
        /// file.
        ///
        /// "Text": The UTF-8 format string that should be displayed. Using this is
        /// not recommended, it is usually better to create text using a localised
        /// text resource.
        ///
        /// "TextColour": The colour of the text. Defaults to white.
        ///
        /// "HorizontalJustification": The justification of the text horizontally.
        /// This can be set to "Left", "Centre" or "Right". Defaults to "Centre".
        ///
        /// "VerticalJustification": The justification of the text vertically. This
        /// can be set to "Bottom", "Centre" or "Top". Defaults to "Centre".
        ///
        /// "AbsCharSpacingOffset": An offset from the standard character spacing in
        /// pixels. This allows characters to be moved appart or closer together.
        /// Defaults to 0.
        ///
        /// "AbsLineSpacingOffset": An offset from the standard line spacing. This
        /// allows lines of text to be moved closer together or further apart.
        /// Defaults to 0.
        ///
        /// "LineSpacingScale": A scale factor to alter the line spacing. This
        /// allows lines of text to be moved closer together or further apart.
        /// Defaults to 1.0
        ///
        /// "MaxNumberOfLines": The maximum number of lines the text will be displayed
        /// across. If 0, there is no limit. Defaults to 0.
        ///
        /// "TextScale": The scale factor that should be used to render the text.
        /// Scaling text significantly reduces the text quality so this should only
        /// be used when absolutely necessary. Defaults to 1.0.
        ///
        /// @author Ian Copland
        //------------------------------------------------------------------------------
        class TextComponent final : public Component
        {
        public:
            CS_DECLARE_NAMEDTYPE(TextComponent);
            //------------------------------------------------------------------------------
            /// Dictionary used to replace [img=Variable] flags with images
            ///
            /// @author Nicolas Tanda
            //------------------------------------------------------------------------------
            using TextIconDictionary = std::unordered_map<std::string, TextIcon>;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The list of properties supported by text.
            //------------------------------------------------------------------------------
            static const std::vector<Core::PropertyMap::PropertyDesc>& GetPropertyDescs();
            //------------------------------------------------------------------------------
            /// Allows querying of whether or not the component implements the interface
            /// associated with the given interface Id. Typically this won't be called
            /// directly, instead the templated version IsA<Interface>() should be used.
            ///
            /// @author Ian Copland
            ///
            /// @param The interface Id.
            ///
            /// @return Whether the object implements the given interface.
            //------------------------------------------------------------------------------
            bool IsA(Core::InterfaceIDType in_interfaceId) const override;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The font that will be used to render the text.
            //------------------------------------------------------------------------------
            const Rendering::FontCSPtr& GetFont() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The localised text resource that will be used to render text.
            //------------------------------------------------------------------------------
            const Core::LocalisedTextCSPtr& GetLocalisedText() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The localised text Id that will be used to render text.
            //------------------------------------------------------------------------------
            const std::string& GetLocalisedTextId() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The UTF-8 format string that will be rendered. This is typically
            /// not used, use of localised text is prefered.
            //------------------------------------------------------------------------------
            const std::string& GetText() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The colour of the text.
            //------------------------------------------------------------------------------
            const Core::Colour& GetTextColour() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The horizontal justification of the text within the owning widget's
            /// bounds.
            //------------------------------------------------------------------------------
            Rendering::HorizontalTextJustification GetHorizontalJustification() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The vertical justification of the text within the owning widget's
            /// bounds.
            //------------------------------------------------------------------------------
            Rendering::VerticalTextJustification GetVerticalJustification() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The absolute pixel offset to the standard character spacing. This is
            /// used to increase of reduce the space between characters in the rendered text.
            //------------------------------------------------------------------------------
            f32 GetAbsoluteCharacterSpacingOffset() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The absolute pixel offset to the standard line spacing spacing. This
            /// is used to increase of reduce the space between lines in the rendered text.
            //------------------------------------------------------------------------------
            f32 GetAbsoluteLineSpacingOffset() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return A scale factor which is applied to the line spacing. This is used to
            /// increase of reduce the space between lines in the rendered text.
            //------------------------------------------------------------------------------
            f32 GetLineSpacingScale() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The maximum number of lines of text that will be rendered. A value
            /// of 0 means there is no limit to the number of lines.
            //------------------------------------------------------------------------------
            s32 GetMaxNumberOfLines() const;
            //------------------------------------------------------------------------------
            /// @author Ian Copland
            ///
            /// @return The text scale. Scaling text significantly reduces the quality of
            /// the text rendered so this should only be used when absolutely necessary.
            //------------------------------------------------------------------------------
            f32 GetTextScale() const;
            //------------------------------------------------------------------------------
            /// @author HMcLaughlin
            ///
            /// @return The minimum text scale that will be used when auto scaling text.
            /// Scaling text significantly reduces the quality of the text rendered so this
            /// should only be used when absolutely necessary.
            //------------------------------------------------------------------------------
            f32 GetMinAutoTextScale() const;
            //------------------------------------------------------------------------------
            /// @author HMcLaughlin
            ///
            /// @return If auto text scaling is enabled
            //------------------------------------------------------------------------------
            bool IsTextAutoScaleEnabled() const;
            //------------------------------------------------------------------------------
            /// Sets the font that will be used to render the text.
            ///
            /// @author Ian Copland
            ///
            /// @param The font.
            //------------------------------------------------------------------------------
            void SetFont(const Rendering::FontCSPtr& in_font);
            //------------------------------------------------------------------------------
            /// Sets the localised text that should be used to render text. A valid
            /// localised text id should also be set.
            ///
            /// @author Ian Copland
            ///
            /// @param The localised text resource.
            //------------------------------------------------------------------------------
            void SetLocalisedText(const Core::LocalisedTextCSPtr& in_localisedText);
            //------------------------------------------------------------------------------
            /// The Id of the entry in the localised text resource that should be used to
            /// render text.
            ///
            /// @author Ian Copland
            ///
            /// @param The localised text Id.
            //------------------------------------------------------------------------------
            void SetLocalisedTextId(const std::string& in_localisedTextId);
            //------------------------------------------------------------------------------
            /// Sets the rendered text from a localised text resource. Text can contain
            /// basic markup for inserting variables into the string. The markup is in the
            /// form [var=VariableName], and the value for the variable is looked up in the
            /// given param dictionary.
            ///
            /// For example "Time remaining: [var=TimeRemaining]" with a param dictionary
            /// containing "TimeRemaining"="12" will become "Time Remaining: 12".
            ///
            /// The markup is also recursive, meaning that the following is valid:
            /// "[var=EntryNumber_[var=Number]]".
            ///
            /// @author Ian Copland
            ///
            /// @param The localised text id which is used to lookup the string in the
            /// localised text resource.
            /// @param The param dictionary which contains the values which should be used
            /// for each variable in the string.
            /// @param The image data dictionary which contains the values which should be
            /// used for each image in the string.
            //------------------------------------------------------------------------------
            void SetLocalisedTextId(const std::string& in_localisedTextId, const Core::ParamDictionary& in_params, const TextIconDictionary& in_imageData = TextIconDictionary());
            //------------------------------------------------------------------------------
            /// Directly sets the text that will be rendered. This is not recommended,
            /// usually is it better to set the text using a localised text resource.
            ///
            /// @author Ian Copland
            ///
            /// @param The text.
            //------------------------------------------------------------------------------
            void SetText(const std::string& in_text);
            //------------------------------------------------------------------------------
            /// Directly sets the text that will be rendered. This is not recommended,
            /// usually is it better to set the text using a localised text resource.
            ///
            /// Will replace flags [img=Variable] with images.
            ///
            /// For example "You have earned 100 [img=Currency]" with a map containing
            /// the data for "Currency" will display an icon for it.
            ///
            /// @author Nicolas Tanda
            ///
            /// @param The text.
            /// @param The image data dictionary which contains the values which should be
            /// used for each image in the string.
            //------------------------------------------------------------------------------
            void SetText(const std::string& in_text, const TextIconDictionary& in_imageData);
            //------------------------------------------------------------------------------
            /// Sets the colour of the rendered text.
            ///
            /// @author Ian Copland
            ///
            /// @param The colour.
            //------------------------------------------------------------------------------
            void SetTextColour(const Core::Colour& in_textColour);
            //------------------------------------------------------------------------------
            /// Sets the horizontal justification of the text within the owning widget's
            /// bounds.
            ///
            /// @author Ian Copland
            ///
            /// @param The justfication.
            //------------------------------------------------------------------------------
            void SetHorizontalJustification(Rendering::HorizontalTextJustification in_horizontalJustification);
            //------------------------------------------------------------------------------
            /// Sets the vertical justification of the text within the owning widget's
            /// bounds.
            ///
            /// @author Ian Copland
            ///
            /// @param The justification.
            //------------------------------------------------------------------------------
            void SetVerticalJustification(Rendering::VerticalTextJustification in_verticalJustification);
            //------------------------------------------------------------------------------
            /// Sets the absolute pixel offset to the standard character spacing. This is
            /// used to increase of reduce the space between characters in the rendered text.
            ///
            /// @author Ian Copland
            ///
            /// @param The offset.
            //------------------------------------------------------------------------------
            void SetAbsoluteCharacterSpacingOffset(f32 in_offset);
            //------------------------------------------------------------------------------
            /// Sets the absolute pixel offset to the standard line spacing spacing. This is
            /// used to increase of reduce the space between lines in the rendered text.
            ///
            /// @author Ian Copland
            ///
            /// @param The offset.
            //------------------------------------------------------------------------------
            void SetAbsoluteLineSpacingOffset(f32 in_offset);
            //------------------------------------------------------------------------------
            /// Sets the scale factor which is applied to the line spacing. This is used to
            /// increase of reduce the space between lines in the rendered text.
            ///
            /// @author Ian Copland
            ///
            /// @param The scale factor.
            //------------------------------------------------------------------------------
            void SetLineSpacingScale(f32 in_scale);
            //------------------------------------------------------------------------------
            /// Sets the maximum number of lines of text that will be rendered. A value of 0
            /// means there is no limit to the number of lines.
            ///
            /// @author Ian Copland
            ///
            /// @param The number of lines, or 0 if no limit.
            //------------------------------------------------------------------------------
            void SetMaxNumberOfLines(s32 in_numLines);
            //------------------------------------------------------------------------------
            /// Sets the text scale. Scaling text significantly reduces the quality of the
            /// text rendered so this should only be used when absolutely necessary.
            ///
            /// @author Ian Copland
            ///
            /// @param The scale factor.
            //------------------------------------------------------------------------------
            void SetTextScale(f32 in_scale);
            //------------------------------------------------------------------------------
            /// Sets the minimum text scale that will be used in scale calculations if
            /// EnableAutoTextScale is true. Scaling text significantly reduces the quality
            /// of the text rendered so this should only be used when absolutely necessary.
            ///
            /// @author HMcLaughlin
            ///
            /// @param in_scale - The scale factor. (Must be <= TextScale)
            //------------------------------------------------------------------------------
            void SetMinAutoTextScale(f32 in_scale);
            //------------------------------------------------------------------------------
            /// Sets if this text component should attempt to auto scale the text to fit
            /// into the labels bounds. This will achieve approximate best case fit scaling
            /// for the current string and label size. This will only attempt to scale
            /// downwards.
            ///
            /// @author HMcLaughlin
            ///
            /// @param in_enable - Enable auto scaling text
            //------------------------------------------------------------------------------
            void SetTextAutoScaleEnabled(bool in_enable);
            
        private:
            friend class ComponentFactory;
            //------------------------------------------------------------------------------
            /// A container for matching Text Icons to their position in the text string.
            ///
            /// @author Ian Copland
            //------------------------------------------------------------------------------
            struct TextIconIndex final
            {
                TextIcon m_icon;
                u32 m_indexInText = 0;
            };
            //------------------------------------------------------------------------------
            /// A container for "cached" Text Icon data used to render the icon as part of
            /// the label.
            ///
            /// Note that the offset combines both the texture atlas "cropped" offset
            /// (if there is one) and the position of the image in the local space of the
            /// label.
            ///
            /// @author Ian Copland
            //------------------------------------------------------------------------------
            struct TextIconCachedData final
            {
                CSRendering::TextureCSPtr m_texture;
                Core::Vector2 m_size;
                Core::Vector2 m_offset;
                Rendering::UVs m_uvs;
            };
            //------------------------------------------------------------------------------
            /// Inserts variables into the given string with the values using basic markup.
            /// The markup is in the form [var=VariableName], and the value for the variable
            /// is looked up in the cached param dictionary.
            ///
            /// For example "Time remaining: [var=TimeRemaining]" with a param dictionary
            /// containing "TimeRemaining"="12" will become "Time Remaining: 12".
            ///
            /// The markup is also recursive, meaning that the following is valid:
            /// "[var=TextValue[var=ValueIndex]]".
            ///
            /// This will also replace any [img= variable] flags with markers and spacing
            /// for the TextComponent to draw icons using the cached images data map.
            ///
            /// @author Nicolas Tanda
            ///
            /// @param in_font - The font which will be used to render the text.
            /// @param in_text - The text containing mark-ups
            /// @param in_params - The param dictionary which contains the values which
            /// should be used for each variable in the string.
            /// @param in_imageData - The image data dictionary which contains the values
            /// which should be used for each image in the string.
            /// @param out_text - [Out] The output text string with the markup resolved.
            /// The string will be initially cleared.
            /// @param out_iconIndices - [Out] The output icons described in markup.
            //------------------------------------------------------------------------------
            static void ReplaceVariables(const Rendering::FontCSPtr& in_font, const std::string& in_text, const Core::ParamDictionary& in_params, const TextIconDictionary& in_imageData,
                                         std::string& out_text, std::vector<TextIconIndex>& out_iconIndices);
            //------------------------------------------------------------------------------
            /// Find any variable or nested variable mark-up and insert the value of the
            /// variables.
            ///
            /// For instance: My string contains [var =a] variable and [var= b] variable
            /// called a and b
            ///
            /// Furthermore, [var= Var_[var= a]_b] has variables called a, and e.g.
            /// Var_12_b (if a == "12")
            ///
            /// This will also replace any [img= variable] flags with actual icons,
            /// using the cached images data map.
            ///
            /// @author Nicolas Tanda
            ///
            /// @param in_font - The font which will be used to render the text.
            /// @param in_params - The param dictionary which contains the values which
            /// should be used for each variable in the string.
            /// @param in_iconDictionary - The image data dictionary which contains the
            /// values which should be used for each image in the string.
            /// @param out_iterator - [OUT] The iterator going through the text
            /// @param out_index - [OUT] The current index of the text
            /// @param out_text - [OUT] The text output
            /// @param out_iconIndices - [Out] The output icons described in markup.
            //------------------------------------------------------------------------------
            static void ReplaceVariablesRecursive(const Rendering::FontCSPtr& in_font, const Core::ParamDictionary& in_params, const TextIconDictionary& in_iconDictionary,
                                                  std::string::const_iterator& out_iterator, u32& out_index, std::string& out_text, std::vector<TextIconIndex>& out_iconIndices);
            //------------------------------------------------------------------------------
            /// Adds the value of the variable with the given name to the output string.
            ///
            /// @author Ian Copland
            ///
            /// @param in_params - The param dictionary which contains all of the different
            /// variables.
            /// @param in_variable - The name of the variable.
            /// @param out_text - [Out] The string to append the variable to.
            ///
            /// @return Whether or not the variable was successfully replaced.
            //------------------------------------------------------------------------------
            static bool AddVariable(const Core::ParamDictionary& in_params, const std::string& in_variableName, std::string& out_text);
            //------------------------------------------------------------------------------
            /// Adds the icon described by the [img] tag to the Text Icon Indices list. Also
            /// makes space in the rendered text string for the icon.
            ///
            /// @author Ian Copland
            ///
            /// @param in_font - The font which will be used to render the text.
            /// @param in_iconDictionary - The icon dictionary.
            /// @param in_iconName - The name of the icon image in the dictionary.
            /// @param in_spaceInfo - Character info on a single space.
            /// @param in_markerInfo - Character info on the marker character.
            /// @param out_index - [Out] The current index of the text
            /// @param out_text - [Out] The string to append space for the icon to.
            /// @param out_iconIndices - [Out] The output list of icons.
            ///
            /// @return Whether or not the icon was successfully added to the label.
            //------------------------------------------------------------------------------
            static bool AddIcon(const Rendering::FontCSPtr& in_font, const TextIconDictionary& in_iconDictionary, const std::string& in_iconName, const Rendering::Font::CharacterInfo& in_spaceInfo,
                                const Rendering::Font::CharacterInfo& in_markerInfo, u32& out_index, std::string& out_text, std::vector<TextIconIndex>& out_iconIndices);
            //------------------------------------------------------------------------------
            /// Builds the cachable Icon data from the list of icon indicies.
            ///
            /// @author Ian Copland
            ///
            /// @param in_font - The font which will be used to render the text.
            /// @param in_builtText - The built text.
            /// @param in_iconIndices - The icon indices.
            /// @param in_textScale - The actual scale factor used to render the text, i.e
            /// After auto-sizing has been taken into account.
            ///
            /// @return The vector of cachable icon data.
            //------------------------------------------------------------------------------
            static std::vector<TextIconCachedData> BuildIcons(const Rendering::FontCSPtr& in_font, const Rendering::CanvasRenderer::BuiltText& in_builtText, const std::vector<TextIconIndex>& in_iconIndices, f32 in_textScale);
            //------------------------------------------------------------------------------
            /// Constructor that builds the text from key-value properties. The properties
            /// used to create the text are described in the class documentation.
            ///
            /// @author Ian Copland
            ///
            /// @param The component name.
            /// @param The property map.
            //------------------------------------------------------------------------------
            TextComponent(const std::string& in_componentName, const Core::PropertyMap& in_properties);
            //------------------------------------------------------------------------------
            /// Render the text using the transform and size of the owning widget.
            ///
            /// @author Ian Copland
            ///
            /// @param The canvas renderer.
            /// @param The final screen space transform.
            /// @param The final screen space size.
            /// @param The final colour.
            //-------------------------------------------------------------------------------
            void OnDraw(Rendering::CanvasRenderer* in_renderer, const Core::Matrix3& in_transform, const Core::Vector2& in_absSize, const Core::Colour& in_absColour) override;
            
            Rendering::FontCSPtr m_font;
            Core::LocalisedTextCSPtr m_localisedText;
            std::string m_localisedTextId;
            std::string m_text;
            Rendering::CanvasRenderer::TextProperties m_textProperties;
            Core::Colour m_textColour;
            std::vector<TextIconIndex> m_iconIndices;
            
            bool m_invalidateCache = true;
            Core::Vector2 m_cachedSize;
            Rendering::CanvasRenderer::BuiltText m_cachedText;
            std::vector<TextIconCachedData> m_cachedIcons;
        };
    }
}

#endif