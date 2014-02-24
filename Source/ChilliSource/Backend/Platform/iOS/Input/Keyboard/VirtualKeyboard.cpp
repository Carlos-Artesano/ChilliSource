//
//  VirtualKeyboard.cpp
//  moFlow
//
//  Created by Scott Downie on 26/11/2010
//  Copyright �2010 Tag Games Limited. All rights reserved.
//

#include <ChilliSource/Backend/Platform/iOS/Input/Keyboard/VirtualKeyboard.h>
#include <ChilliSource/Backend/Platform/iOS/Input/Keyboard/VirtualKeyboardDelegate.h>

#include <ChilliSource/Core/String/StringUtils.h>

VirtualKeyboardDelegate* gpDelegate = nil;

namespace ChilliSource
{
	namespace iOS
	{
		//-------------------------------------------
		/// Constructor
		///
		/// Create and hide the text view
		//-------------------------------------------
		CVirtualKeyboard::CVirtualKeyboard() : mpTextView(nil)
		{
            if(!gpDelegate)
            {
                gpDelegate = [[VirtualKeyboardDelegate alloc] initWithKeyboard:this];
            }
            
			if(!mpTextView)
			{
				//Just create it with garbage size as we will never see it
				mpTextView = [[UITextField alloc] initWithFrame:CGRectMake(200, 10, 100, 200)];
				[mpTextView setReturnKeyType:UIReturnKeyDone];
				mpTextView.autocorrectionType = UITextAutocorrectionTypeNo;
                mpTextView.autocapitalizationType = UITextAutocapitalizationTypeNone;
				mpTextView.keyboardType = UIKeyboardTypeASCIICapable;
                mpTextView.delegate = gpDelegate;
			}
		}
		//-------------------------------------------
		/// Show 
		///
		/// Display the keyboard
		/// Triggers OnKeyboardShow event
		///
		/// @param Default text 
		//-------------------------------------------
		void CVirtualKeyboard::Show()
		{
			if(!mbIsActive && [mpTextView canBecomeFirstResponder])
			{
				//Show the keyboard!
				[[EAGLView sharedInstance] addSubview:mpTextView];
				mpTextView.hidden = YES;
				[mpTextView becomeFirstResponder];
				mbIsActive = true;
				
				//Notify our listeners
				mOnKeyboardShowEvent.Invoke();
			}
		}
		//-------------------------------------------
		/// Update Text
		///
		/// @param New Text
        /// @return Whether to accept the text
		//-------------------------------------------
		bool CVirtualKeyboard::UpdateText(NSString* inpstrText)
		{
            //Inform subscripted to this event and receive if text was rejected lower down
            bool bRejectText = false;
            
            mOnKeyboardTextChangeEvent.Invoke<const Core::UTF8String&, bool*>(Core::CStringUtils::NSStringToUTF8String(inpstrText), &bRejectText);
            
            if(!bRejectText)
            {
                mstrText = Core::CStringUtils::NSStringToUTF8String(inpstrText);
            }
            
            return !bRejectText;
		}
		//-------------------------------------------
		/// Hide 
		///
		/// Remove the keyboard
		/// Triggers OnKeyboardHide event
		//-------------------------------------------
		void CVirtualKeyboard::Hide()
		{
			if(mbIsActive)
			{
				[mpTextView resignFirstResponder];
				[mpTextView removeFromSuperview];
				mbIsActive = false;
				
				//Notify our listeners
				mOnKeyboardHideEvent.Invoke();
				
				mstrText.clear();
				mpTextView.text = @"";
			}
		}
        //-------------------------------------------
        /// Set Text 
        ///
        /// @param Text cache for keyboard
        //-------------------------------------------
		void CVirtualKeyboard::SetText(const Core::UTF8String& instrText)
        {
            mstrText = instrText;
            mpTextView.text = Core::CStringUtils::UTF8StringToNSString(mstrText);
        }
        //-------------------------------------------
        /// SetKeyboardType
        ///
        /// @param keyboard type to be set
        //-------------------------------------------
        void CVirtualKeyboard::SetKeyboardType(Input::KeyboardType ineKeyboardType)
        {
            if(!mpTextView)
            	return;

            switch (ineKeyboardType)
            {
                case Input::KeyboardType::k_text:
                    mpTextView.keyboardType = UIKeyboardTypeASCIICapable;
                    break;
                case Input::KeyboardType::k_numeric:
                    mpTextView.keyboardType = UIKeyboardTypeNumberPad;
                    break;
                default:
                	ERROR_LOG("Invalid keyboard type passed to virtual keyboard!");
                	mpTextView.keyboardType = UIKeyboardTypeASCIICapable;
                    break;
            }
        }
        //------------------------
        /// Set Capitalisation Method
        ///
        /// @param Requests keyboard Capitalisation Method to show
        //------------------------
        void CVirtualKeyboard::SetCapitalisationMethod(Input::KeyboardCapitalisation ineKeyboardCapitalisation)
        {
            if(!mpTextView)
            	return;
            
            switch (ineKeyboardCapitalisation)
            {
                case Input::KeyboardCapitalisation::k_none:
                    mpTextView.autocapitalizationType = UITextAutocapitalizationTypeNone;
                    break;
                case Input::KeyboardCapitalisation::k_words:
                    mpTextView.autocapitalizationType = UITextAutocapitalizationTypeWords;
                    break;
                case Input::KeyboardCapitalisation::k_sentences:
                    mpTextView.autocapitalizationType = UITextAutocapitalizationTypeSentences;
                    break;
                case Input::KeyboardCapitalisation::k_all:
                    mpTextView.autocapitalizationType = UITextAutocapitalizationTypeAllCharacters;
                    break;
                default:
                    WARNING_LOG("Unknown Capitalisation Method");
            }
        }
		//--------------------------------------------
		/// Destructor
		//--------------------------------------------
		CVirtualKeyboard::~CVirtualKeyboard()
		{
			if(mpTextView)
			{
				[mpTextView release];
				mpTextView = nil;
			}
            
            if(gpDelegate)
            {
                [gpDelegate release];
                gpDelegate = nil;
            }
		}
	}
}
