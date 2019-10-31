#include "CreateResourceContext.h"
#include "pthread.h"
#include "../Image.h"
#include "../core/NativeTexture.h"
#include "../core/oxygine.h"

namespace oxygine
{
    LoadResourcesContext* LoadResourcesContext::get()
    {
        LoadResourcesContext* scontext = &SingleThreadResourcesContext::instance;
        return scontext;
    }

    CreateTextureTask::CreateTextureTask(): linearFilter(true), clamp2edge(true)
    {
    }

    void CreateTextureTask::ready() const
    {
        if (!linearFilter)
            dest->setLinearFilter(linearFilter);
        if (!clamp2edge)
            dest->setClamp2Edge(clamp2edge);
    }

    XmlWalker::XmlWalker(
        const std::string* xmlFolder,
        const std::string& path,
        float scaleFactor,
        bool load,
        bool alpha,
        pugi::xml_node xml) :
        _root(xml),
        _notStarted(true),
        _scaleFactor(scaleFactor),
        _load(load),
        _alphaHitTest(alpha),
        _xmlFolder(xmlFolder),
        _path(path)
    {
        //_alphaTracking = true;
    }

    const char* isRelative(const char* str)
    {
        const char* str_ = str;
        if (*str == '.')
        {
            ++str;
            if (!*str || *str == '\\' || *str == '/')
                return str;
        }

        return 0;
    }

    std::string XmlWalker::connectPath(const char* currentPath, const char* str)
    {
        std::string s;

        const char* rl = isRelative(str);
        if (rl)
        {
            s = currentPath;
            s += rl;
            return s;
        }
        return str;
    }


    std::string XmlWalker::getPath(const char* attrName) const
    {
        const char* str = _root.attribute(attrName).as_string();
        const char* rl = isRelative(str);
        if (rl)
        {
            return *_xmlFolder + rl;
        }

        return _path + str;
    }

    XmlWalker XmlWalker::next()
    {
        while (1)
        {
            if (_notStarted)
            {
                _last = _root.first_child();
                _notStarted = false;
            }
            else
            {
                _last = _last.next_sibling();
            }

            const char* name = _last.name();
            pugi::xml_attribute attr;

            if (!strcmp(name, "set"))
            {
                _checkSetAttributes(_last);
                continue;
            }

            break;
        }

        return XmlWalker(_xmlFolder, _path, _scaleFactor, _load, _alphaHitTest, _last);
    }

    void XmlWalker::_checkSetAttributes(pugi::xml_node node)
    {
        pugi::xml_attribute attr = node.first_attribute();
        while (attr)
        {
            if (!strcmp(attr.name(), "path"))
            {
                _path = connectPath(_xmlFolder->c_str(), attr.value());
                if (!_path.empty())
                    _path += "/";
            }
            else if (!strcmp(attr.name(), "load"))
            {
                _load = attr.as_bool();
            }
            else if (!strcmp(attr.name(), "scale_factor"))
            {
                _scaleFactor = attr.as_float(1.0f);
            }
            else if (!strcmp(attr.name(), "hit_test"))
            {
                _alphaHitTest = attr.as_bool(false);
            }

            attr = attr.next_attribute();
        }
    }

    void XmlWalker::checkSetAttributes()
    {
        _checkSetAttributes(_root);
    }

    RestoreResourcesContext RestoreResourcesContext::instance;
    void RestoreResourcesContext::createTexture(const CreateTextureTask& opt)
    {
        opt.dest->init(opt.src->lock(), false);
        opt.ready();
    }

    bool RestoreResourcesContext::isNeedProceed(spNativeTexture t)
    {
        return true;
    }

    SingleThreadResourcesContext SingleThreadResourcesContext::instance;

    void SingleThreadResourcesContext::createTexture(const CreateTextureTask& opt)
    {
        opt.dest->init(opt.src->lock(), false);
        opt.ready();
    }

    bool SingleThreadResourcesContext::isNeedProceed(spNativeTexture t)
    {
        return t->getHandle() == 0;
    }
}
