/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_TEMPLATETHUMBNAILSCOMPONENT_H_INCLUDED
#define JUCER_TEMPLATETHUMBNAILSCOMPONENT_H_INCLUDED

//=====================================================================================================
/**
    Template option tile button.
    The drawable button object class for the tile icons and buttons in the TemplateTileBrowser
*/
class TemplateOptionButton   : public DrawableButton
{
public:
    TemplateOptionButton (const String& buttonName, ButtonStyle buttonStyle, const char* thumbSvg)
       : DrawableButton (buttonName, buttonStyle)
    {
        // svg for thumbnail icon
        ScopedPointer<XmlElement> svg (XmlDocument::parse (thumbSvg));
        assert (svg != nullptr);

        thumb = Drawable::createFromSVG (*svg);

        // svg for thumbnail background highlight
        ScopedPointer<XmlElement> backSvg (XmlDocument::parse (BinaryData::iconHighlight_svg));
        assert (backSvg != nullptr);

        hoverBackground = Drawable::createFromSVG (*backSvg);

        name = buttonName;

        description = "<insert description>";
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool /*isButtonDown*/) override
    {
        const Rectangle<float> bounds (getLocalBounds().toFloat());
        const Colour buttonColour (0xfff29300);

        if (isMouseOverButton)
        {
            if (getStyle() == ButtonStyle::ImageFitted)
            {
                hoverBackground->drawWithin (g, bounds, RectanglePlacement::centred, 1.0);
                thumb->drawWithin (g, bounds, RectanglePlacement::centred, 1.0);
            }
            else
            {
                g.setColour (buttonColour.withAlpha (0.3f));
                g.fillRoundedRectangle (bounds.reduced (2.0f, 2.0f), 10.0f);
                g.setColour (buttonColour);
                g.drawRoundedRectangle (bounds.reduced (2.0f, 2.0f), 10.0f, 2.0f);
            }
        }
        else
        {
            if (getStyle() == ButtonStyle::ImageFitted)
            {
                thumb->drawWithin (g, bounds, RectanglePlacement::centred, 1.0);
            }
            else
            {
                g.setColour (buttonColour);
                g.drawRoundedRectangle (bounds.reduced (2.0f, 2.0f), 10.0f, 2.0f);
            }
        }

        Rectangle<float> textTarget;

        // center the text for the text buttons or position the text in the image buttons
        if (getStyle() != ButtonStyle::ImageFitted)
        {
            textTarget = getLocalBounds().toFloat();
        }
        else
        {
            textTarget = RectanglePlacement (RectanglePlacement::centred).appliedTo (thumb->getDrawableBounds(), bounds);
            textTarget = textTarget.removeFromBottom (textTarget.getHeight() * 0.3f);
        }

        g.setColour (Colours::white);
        g.drawText (name, textTarget, Justification::centred, true);
    }

    void resized() override
    {
        thumb->setBoundsToFit (0, 0, getWidth(), getHeight(), Justification::centred, false);
    }

    void setDescription (String descript) noexcept
    {
        description = descript;
    }

    String getDescription() const noexcept
    {
        return description;
    }

private:
    ScopedPointer<Drawable> thumb, hoverBackground;
    String name, description;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateOptionButton)
};



//=====================================================================================================
/**
    Project Template Component for front page.
    Features multiple icon buttons to select the type of project template
*/
class TemplateTileBrowser   : public Component,
                              private Button::Listener
{
public:
    TemplateTileBrowser (NewProjectWizardClasses::WizardComp* projectWizard)
    {
        addOptionButton ("GUI Application",      BinaryData::iconGui_svg,       "Creates a blank JUCE application with a single window component.");
        addOptionButton ("Audio Application",    BinaryData::iconAudio_svg,     "Creates a blank JUCE application with a single window component and Audio and MIDI in/out functions.");
        addOptionButton ("Audio Plug-in",        BinaryData::iconPlugin_svg,    "Creates a VST or AU audio plug-in for use within a host program. This template features a single window component and Audio/MIDI IO functions");
        addOptionButton ("Animated Application", BinaryData::iconAnimation_svg, "Creates a blank JUCE application with a single window component that updates and draws at 60fps.");
        addOptionButton ("Opengl Application",   BinaryData::iconOpengl_svg,    "Creates a blank JUCE application with a single window component. This component supports all OPENGL drawing features including 3D model import and glsl shaders.");
        addOptionButton ("Console Application",  BinaryData::iconConsole_svg,   "Creates a blank console application with support for all JUCE features.");
        addOptionButton ("Static Library",       BinaryData::iconStatic_svg,    "Creates a Static Library template with support for all JUCE features");
        addOptionButton ("Dynamic Library",      BinaryData::iconDynamic_svg,   "Creates a Dynamic Library template with support for all JUCE features");

        // Handle Open Project button functionality
        ApplicationCommandManager& commandManager = IntrojucerApp::getCommandManager();

        blankProjectButton = new TemplateOptionButton ("Create Blank Project", TemplateOptionButton::ButtonStyle::ImageOnButtonBackground, BinaryData::iconOpenfile_svg);
        openProjectButton  = new TemplateOptionButton ("Open Existing Project", TemplateOptionButton::ButtonStyle::ImageOnButtonBackground, BinaryData::iconOpenfile_svg);
        openProjectButton->setCommandToTrigger (&commandManager, CommandIDs::open, true);

        exampleProjectButton = new TemplateOptionButton ("Open Example Project", TemplateOptionButton::ButtonStyle::ImageOnButtonBackground, BinaryData::iconOpenfile_svg);
        exampleProjectButton->setCommandToTrigger (&commandManager, CommandIDs::open, true);

        addAndMakeVisible (blankProjectButton);
        addAndMakeVisible (openProjectButton);
        addAndMakeVisible (exampleProjectButton);

        newProjectWizard = projectWizard;
    }

    void addOptionButton (const char* name, const char* svg, const char* desc)
    {
        TemplateOptionButton* b = new TemplateOptionButton (name, TemplateOptionButton::ButtonStyle::ImageFitted, svg);
        optionButtons.add (b);
        addAndMakeVisible (b);
        b->setDescription (desc);
        b->addListener (this);
    }

    void paint (Graphics& g) override
    {
        g.setColour (Colours::black.withAlpha (0.2f));
        g.fillRect (0, 0, getWidth(), 60);

        g.setColour (Colours::white);
        g.setFont (20);
        g.drawText ("Create New Project", 0, 0, getWidth(), 60, Justification::centred, true);

        // draw the descriptions of each template if hovered;
        // (repaint is called by the button listener on change state)
        Rectangle<int> descriptionBox = getBounds().reduced (30, 30);
        descriptionBox = descriptionBox.removeFromBottom (50);

        g.setColour (Colours::white.withAlpha (0.4f));
        g.setFont (15);

        for (int i = 0; i < 8; ++i)
            if (optionButtons.getUnchecked(i)->getState() == TemplateOptionButton::ButtonState::buttonOver)
                g.drawFittedText (optionButtons.getUnchecked(i)->getDescription(), descriptionBox, Justification::centred, 5, 1.0f);
    }

    void resized() override
    {
        Rectangle<int> allOpts = getBounds().reduced (40, 60);
        allOpts.removeFromBottom (allOpts.getHeight() * 0.25);

        const int numHorizIcons = 4;
        const int optStep = allOpts.getWidth()/numHorizIcons;

        for (int i = 0; i < optionButtons.size(); ++i)
        {
            const int yShift = i < numHorizIcons ? 0 : 1;

            optionButtons.getUnchecked(i)->setBounds (Rectangle<int> (allOpts.getX() + (i % numHorizIcons) * optStep,
                                                                      allOpts.getY() + yShift * allOpts.getHeight() / 2,
                                                                      optStep, allOpts.getHeight() / 2)
                                                        .reduced (10, 10));
        }

        Rectangle<int> openButtonBounds = getBounds();
        openButtonBounds.removeFromBottom (proportionOfHeight (0.12f));
        openButtonBounds = openButtonBounds.removeFromBottom (120);
        openButtonBounds.reduce (50, 40);

        blankProjectButton->setBounds (openButtonBounds.removeFromLeft (optStep - 20));
        exampleProjectButton->setBounds (openButtonBounds.removeFromRight (optStep - 20));
        openProjectButton->setBounds (openButtonBounds.reduced (18, 0));
    }

    static int getIndexOfButton (const String& buttonText)
    {
        if (buttonText == "GUI Application")        return 0;
        if (buttonText == "Console Application")    return 1;
        if (buttonText == "Audio Plug-in")          return 2;
        if (buttonText == "Static Library")         return 3;
        if (buttonText == "Dynamic Library")        return 4;

        // new templates without actual templates yet
        if (buttonText == "Animated Application")   return 0;
        if (buttonText == "Audio Application")      return 0;
        if (buttonText == "Opengl Application")     return 0;

        jassertfalse;
        return 0;
    }

    void buttonClicked (Button* b) override
    {
        newProjectWizard->projectType.setSelectedItemIndex (getIndexOfButton (b->getButtonText()));

        if (SlidingPanelComponent* parent = findParentComponentOfClass<SlidingPanelComponent>())
        {
            if (parent->getNumTabs() > 0 && b->getButtonText() != "Open Existing Project")
                parent->goToTab (parent->getCurrentTabIndex() + 1);
        }
        else
        {
            jassertfalse;
        }
    }

    void buttonStateChanged (Button*) override
    {
        repaint();
    }

private:
    OwnedArray<TemplateOptionButton> optionButtons;
    NewProjectWizardClasses::WizardComp* newProjectWizard;
    ScopedPointer<TemplateOptionButton> blankProjectButton, openProjectButton, exampleProjectButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateTileBrowser)
};


#endif  // JUCER_TEMPLATETHUMBNAILSCOMPONENT_H_INCLUDED
