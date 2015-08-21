/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static inline juce::Colour tojColor(std::array<float, 4> const& color)
{
    return juce::Colour::fromFloatRGBA(color[0], color[1], color[2], color[3]);
}

static inline juce::Path fromPath(vector<t_pt> const& points)
{
    juce::Path path;
    for(int i = 0; i < points.size(); i++)
    {
        if(points[i].x == E_PATH_MOVE && i < points.size() - 1)
        {
            path.startNewSubPath(points[i+1].x, points[i+1].y);
            ++i;
        }
        else if(points[i].x == E_PATH_LINE && i < points.size() - 1)
        {
            path.lineTo(points[i+1].x, points[i+1].y);
            ++i;
        }
        else if(points[i].x == E_PATH_CURVE && i < points.size() - 3)
        {
            path.cubicTo(points[i+1].x, points[i+1].y,
                         points[i+2].x, points[i+2].y,
                         points[i+3].x, points[i+3].y);
            i += 3;
        }
        else if(points[i].x == E_PATH_CLOSE)
        {
            path.closeSubPath();
        }
    }
    return path;
}

static inline juce::Path fromRect(vector<t_pt> const& points)
{
    juce::Path path;
    if(points.size() == 5)
    {
        path.addRectangle(points[0].x, points[0].y,
                          points[2].x - points[0].x,
                          points[2].y - points[0].y);
    }
    else
    {
        path.addRoundedRectangle(points[6].x, points[0].y,
                                 points[2].x - points[6].x,
                                 points[5].y - points[0].y,
                                 points[2].y - points[0].y);
    }
    return path;
}

static inline juce::Path fromOval(vector<t_pt> const& points)
{
    juce::Path path;
    path.addEllipse(points[0].x, points[0].y,
                    points[1].x - points[0].x,
                    points[1].y - points[0].y);
    return path;
}

static inline juce::Path tojPath(Gobj const& obj)
{
    if(obj.type() == Gobj::Path)
    {
        return fromPath(obj.points());
    }
    else if(obj.type() == Gobj::Rect)
    {
        return fromRect(obj.points());
    }
    else if(obj.type() == Gobj::Oval)
    {
        return fromOval(obj.points());
    }
    else if(obj.type() == Gobj::Arc)
    {
        
    }
    return juce::Path();
}

// ==================================================================================== //
//                                  MENU INTERFACE                                      //
// ==================================================================================== //

StringArray MenuInterface::getMenuBarNames()
{
    StringArray menus;
    menus.add("File");
    menus.add("Preset");
    menus.add("Console");
    menus.add("Help");
    return menus;
}

PopupMenu MenuInterface::getMenuForIndex(int topLevelMenuIndex, const String& menuName)
{
    PopupMenu menu;
    switch(topLevelMenuIndex)
    {
        case 0:
            menu.addItem(0, String("Open..."));
            menu.addItem(1, String("Open Recent"));
            menu.addItem(2, String("Close"));
            break;
            
        default:
            break;
    }
    return menu;
}

// ==================================================================================== //
//                                  OBJECT INTERFACE                                    //
// ==================================================================================== //

ObjectInterface::ObjectInterface(CamomileInterface& camo, sGui object) :
m_interface(camo),
m_object(object),
m_messenger(make_shared<Messenger>(object->getBindingName())),
m_attached(false)
{
    if(object)
    {
        const std::array<int,2> bounds = object->getSize();
        const int offset = object->getBorderSize() * 2;
        Component::setSize(bounds[0] + offset, bounds[1] + offset);
        Component::setVisible(true);
        Component::setInterceptsMouseClicks(object->wantMouse(), object->wantMouse());
        Component::setMouseClickGrabsKeyboardFocus(object->wantKeyboard());
        Component::setWantsKeyboardFocus(object->wantKeyboard());
    }
}

void ObjectInterface::paint(Graphics& g)
{
    sGui object = m_object.lock();
    if(object)
    {
        if(!m_attached)
        {
            m_messenger->addListener(this);
            m_attached = true;
        }
        m_interface.lock();
        const int offset = object->getBorderSize();
        const AffineTransform transform(AffineTransform::translation(offset, offset));
        g.fillAll(tojColor(object->getBackgroundColor()));
        std::vector<Layer> layers(object->paint());
        for(auto it : layers)
        {
            if(it.state() == Layer::Todraw)
            {
                for(int i = 0; i < it.size(); i++)
                {
                    const Gobj obj = it[i];
                    g.setColour(tojColor(obj.color()));
                    if(obj.type() == Gobj::Text)
                    {
                        vector<t_pt> const points(obj.points());
                        g.setFont(obj.fontSize());
                        g.drawText(juce::String(obj.text()),
                                   points[0].x,
                                   points[0].y,
                                   points[1].x,
                                   points[1].y,
                                   juce::Justification(obj.justification()), obj.wrapText());
                    }
                    else if(obj.filled())
                    {
                        g.fillPath(tojPath(obj), transform);
                    }
                    else
                    {
                        g.strokePath(tojPath(obj), PathStrokeType(obj.witdh()), transform);
                    }
                }
                it.close();
            }
        }
        g.setColour(tojColor(object->getBorderColor()));
        g.drawRect(getBounds().withZeroOrigin(), object->getBorderSize());
        m_interface.unlock();
    }
    else
    {
        g.fillAll(Colours::whitesmoke);
    }
}

void ObjectInterface::mouseMove(const MouseEvent& event)
{
    sGui object = m_object.lock();
    if(object)
    {
        object->mouseMove({float(event.x), float(event.y)}, event.mods.getRawFlags());
    }
}

void ObjectInterface::mouseEnter(const MouseEvent& event)
{
    setMouseCursor(juce::MouseCursor::NormalCursor);
    sGui object = m_object.lock();
    if(object)
    {
        object->mouseEnter({float(event.x), float(event.y)}, event.mods.getRawFlags());
    }
}

void ObjectInterface::mouseExit(const MouseEvent& event)
{
    sGui object = m_object.lock();
    if(object)
    {
        object->mouseExit({float(event.x), float(event.y)}, event.mods.getRawFlags());
    }
}

void ObjectInterface::mouseDown(const MouseEvent& event)
{
    sGui object = m_object.lock();
    if(object)
    {
        object->mouseDown({float(event.x), float(event.y)}, event.mods.getRawFlags());
    }
}

void ObjectInterface::mouseDrag(const MouseEvent& event)
{
    sGui object = m_object.lock();
    if(object)
    {
        object->mouseDrag({float(event.x), float(event.y)}, event.mods.getRawFlags());
    }
}

void ObjectInterface::mouseUp(const MouseEvent& event)
{
    sGui object = m_object.lock();
    if(object)
    {
        object->mouseUp({float(event.x), float(event.y)}, event.mods.getRawFlags());
    }
}

void ObjectInterface::mouseDoubleClick(const MouseEvent& event)
{
    sGui object = m_object.lock();
    if(object)
    {
        object->mouseDoubleClick({float(event.x), float(event.y)}, event.mods.getRawFlags());
    }
}

void ObjectInterface::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
    sGui object = m_object.lock();
    if(object)
    {
        object->mouseWheelMove({float(event.x), float(event.y)}, event.mods.getRawFlags(), {wheel.deltaX, wheel.deltaY});
    }
}

bool ObjectInterface::keyPressed(const KeyPress& key)
{
    sGui object = m_object.lock();
    if(object)
    {
        char buffer[MB_CUR_MAX];
        wctomb(buffer, key.getTextCharacter());
        if(key.getKeyCode() == KeyPress::deleteKey ||
           key.getKeyCode() == KeyPress::returnKey ||
           key.getKeyCode() == KeyPress::tabKey ||
           key.getKeyCode() == KeyPress::escapeKey)
        {
            
            if(key.getKeyCode() == KeyPress::deleteKey)
            {
                object->keyFilter(buffer[0], EKEY_DEL);
            }
            else if(key.getKeyCode() == KeyPress::returnKey)
            {
                object->keyFilter(buffer[0], EKEY_ENTER);
            }
            else if(key.getKeyCode() == KeyPress::tabKey)
            {
                object->keyFilter(buffer[0], EKEY_TAB);
            }
            else if(key.getKeyCode() == KeyPress::escapeKey)
            {
                object->keyFilter(buffer[0], EKEY_ESC);
            }
        }
        else
        {
            object->keyPressed(buffer[0], key.getModifiers().getRawFlags());
        }
        return true;
    }
    return false;
}

void ObjectInterface::receive(const std::string& dest, t_symbol* s)
{
    if(isShowing() && s == s_cream_repaint)
    {
        const MessageManagerLock thread(Thread::getCurrentThread());
        if(thread.lockWasGained())
        {
             repaint();
        }
    }
}

void ObjectInterface::receive(const std::string& dest, t_symbol* s, std::vector<const t_atom *> atoms)
{
    sGui object = m_object.lock();
    if(object && isShowing() &&
       s == s_cream_texteditor &&
       atoms.size() == 2 && atom_gettype(atoms[0]) == A_SYMBOL && atom_gettype(atoms[1]) == A_FLOAT)
    {
        const t_etexteditor* editor = etexteditor_getfromsymbol(atoms[0]->a_w.w_symbol);
        const ewidget_action action = ewidget_action(atoms[1]->a_w.w_float);
        const String name(editor->c_name->s_name);
        if(editor)
        {
            const MessageManagerLock thread(Thread::getCurrentThread());
            if(thread.lockWasGained())
            {
                switch(action)
                {
                    case EWIDGET_CREATE:
                        m_editors.add(new TextEditor(name));
                        break;
                    case EWIDGET_DESTROY:
                    {
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                                m_editors.remove(i);
                                break;
                            }
                        }
                    }
                        break;
                    case EWIDGET_SETTEXT:
                    {
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                                if(editor->c_text && editor->c_size)
                                {
                                    m_editors[i]->setText(String());
                                    break;
                                }
                                else
                                {
                                    m_editors[i]->clear();
                                    break;
                                }
                            }
                        }
                    }
                        break;
                    case EWIDGET_GETTEXT:
                        ;
                        break;
                    case EWIDGET_CLEAR:
                    {
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                                m_editors[i]->clear();
                                break;
                            }
                        }
                    }
                        break;
                    case EWIDGET_SETFONT:
                    {
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                                m_editors[i]->setFont(juce::Font(editor->c_font.c_size));
                                break;
                            }
                        }
                    }
                        break;
                    case EWIDGET_SETBGCOLOR:
                    {
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                                m_editors[i]->setColour(TextEditor::backgroundColourId, Colour::fromFloatRGBA(editor->c_bgcolor.red, editor->c_bgcolor.green, editor->c_bgcolor.blue, editor->c_bgcolor.alpha));
                                break;
                            }
                        }
                    }
                        break;
                    case EWIDGET_SETTXTCOLOR:
                    {
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                                m_editors[i]->setColour(TextEditor::textColourId, Colour::fromFloatRGBA(editor->c_txtcolor.red, editor->c_txtcolor.green, editor->c_txtcolor.blue, editor->c_txtcolor.alpha));
                                break;
                            }
                        }
                    }
                        break;
                    case EWIDGET_WRAPMODE:
                    {
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                               m_editors[i]->setMultiLine(bool(editor->c_wrap), bool(editor->c_wrap));
                                break;
                            }
                        }
                    }
                        break;
                    case EWIDGET_POPUP:
                    {
                        const int offset = object->getBorderSize();
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                                m_editors[i]->setBounds(int(editor->c_bounds.x + offset),
                                                        int(editor->c_bounds.y + offset),
                                                        int(editor->c_bounds.width),
                                                        int(editor->c_bounds.height));
                                addAndMakeVisible(m_editors[i]);
                                break;
                            }
                        }
                    }
                        break;
                    case EWIDGET_GRABFOCUS:
                    {
                        for(int i = 0; i < m_editors.size(); i++)
                        {
                            if(m_editors[i]->getName() == name)
                            {
                                m_editors[i]->grabKeyboardFocus();
                                break;
                            }
                        }
                    }
                        break;
                        
                    default:
                        break;
                }
                
            }
        }
    }
}

// ==================================================================================== //
//                                  CAMOMILE INTERFACE                                  //
// ==================================================================================== //

CamomileInterface::CamomileInterface(CamomileAudioProcessor& p) : AudioProcessorEditor(&p),
    m_processor(p),
    m_dropping(false)
{
    m_processor.addListener(this);
    patchChanged();
    setSize(600, 400);
    Component::setWantsKeyboardFocus(true);
}

CamomileInterface::~CamomileInterface()
{
    m_processor.removeListener(this);
}

void CamomileInterface::paint(Graphics& g)
{
    m_processor.lock();
    shared_ptr<const Patch> patch = m_processor.getPatch();
    if(patch)
    {
        sGui camo = patch->getCamomile();
        if(camo)
        {
            g.fillAll(tojColor(camo->getBackgroundColor()));
            g.setColour(tojColor(camo->getBorderColor()));
            g.drawRect(getBounds().withZeroOrigin());
        }
        else
        {
            g.fillAll(Colours::white);
            g.setColour(Colours::black);
            g.setFont (15.0f);
            g.drawText(juce::String("The patch is not valid !"), getBounds().withZeroOrigin(), juce::Justification::centred);
        }
        m_processor.unlock();
    }
    else
    {
        m_processor.unlock();
        g.fillAll(Colours::white);
        g.setColour(Colours::black);
        g.setFont (15.0f);
        g.drawText(juce::String("Drag & Drop your patch..."), getBounds().withZeroOrigin(), juce::Justification::centred);
    }
    
    if(m_dropping)
    {
        g.fillAll(Colours::lightblue.withAlpha(0.2f));
    }
}

bool CamomileInterface::isInterestedInFileDrag(const StringArray& files)
{
    if(files.size())
    {
        for(int i = 0; i < files.size(); i++)
        {
            if(files[i].endsWith(juce::StringRef(".pd")))
            {
                return true;
            }
        }
    }
    return false;
}

void CamomileInterface::filesDropped(const StringArray& files, int x, int y)
{
    if(files.size())
    {
        for(int i = 0; i < files.size(); i++)
        {
            juce::File file(files[i]);
            if(file.getFileExtension() == juce::String(".pd"))
            {
                m_processor.loadPatch(file);
            }
        }
    }
}

void CamomileInterface::patchChanged()
{
    removeAllChildren();
    m_objects.clear(true);
    m_processor.lock();
    shared_ptr<const Patch> patch = m_processor.getPatch();
    if(patch)
    {
        
        sGui camo = patch->getCamomile();
        if(camo)
        {
            const std::vector<sGui> objects = patch->getGuis();
            const std::array<int,2> ref = camo->getPosition();
            for(auto it : objects)
            {
                ObjectInterface* inte = m_objects.add(new ObjectInterface(*this, it));
                const std::array<int,2> pos = it->getPosition();
                const int offset = it->getBorderSize();
                inte->setTopLeftPosition(pos[0] - ref[0] - offset, pos[1] - ref[1] - offset);
                addChildComponent(inte);
            }
        }
    }
    m_processor.unlock();
    const MessageManagerLock mmLock;
    if(mmLock.lockWasGained())
    {
        repaint();
    }
}

void CamomileInterface::fileDragEnter(const StringArray& files, int x, int y)
{
    const MessageManagerLock mmLock;
    if(mmLock.lockWasGained())
    {
        m_dropping = true;
        repaint();
    }
}

void CamomileInterface::fileDragExit(const StringArray& files)
{
    m_dropping = false;
    repaint();
}

void CamomileInterface::resized()
{
    
}
