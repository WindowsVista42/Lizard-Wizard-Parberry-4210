#include "Helpers.h"
#include "Game.h"

std::string XMLFindItem(tinyxml2::XMLElement* xml, const char* group_tag, const char* item_tag, const char* name) {
    // NOTE(sean): LSpriteRenderer::LoadByIndex() used as ref

    if (xml == 0) { ABORT("Unable to find gamesettings.xml"); }

    XMLElement* group = xml->FirstChildElement(group_tag);
    ABORT_EQ_FORMAT(group, 0, "Unable to find group tag \"%s\"", group_tag);

    std::string path(group->Attribute("path"));
    XMLElement* item = group->FirstChildElement(item_tag);

    // find specific tag
    while (item != 0 && strcmp(name, item->Attribute("name"))) {
        item = item->NextSiblingElement(item_tag);
    }

    // was not found
    ABORT_EQ_FORMAT(item, 0, "Unable to find item tag \"%s\"", item_tag);

    if (item->Attribute("file")) {
        return path + "\\" + item->Attribute("file");
    } else {
        return "";
    }
}