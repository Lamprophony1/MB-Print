#ifndef INCLUDE_MBCOREUTILS_TOOLPATH_TAGS_H_
#define INCLUDE_MBCOREUTILS_TOOLPATH_TAGS_H_

namespace MakerBot {

enum ToolpathTag : unsigned int {
    kAnchor = 0,
    kBacklashCompensation,
    kBridge,
    kConnection,
    kFlickMove,  //Not sure why there is Flick and FlickMove
    kInfill,
    kInset,
    kOutline,
    kLeakyTravelMove,
    kTravelMove,
    kTrailingExtrusionMove,
    kSpur,
    kSupport,
    kRetract,
    kRestart,
    kFlick,
    kPurge,
    kLongRetract,
    kLongRestart,
    kBeadModeExternal,
    kBeadModeExternalThick,
    kBeadModeExternalThicker,
    kBeadModeExternalThickest,
    kBeadModeInternal,
    kBeadModeInternalThick,
    kBeadModeInternalThicker,
    kBeadModeInternalThickest,
    kBeadModeUser1,
    kBeadModeUser2,
    kBeadModeUser3,
    kBeadModeUser4,
    LAST
};

inline std::string stringFromToolpathTag(const ToolpathTag tag) {
    switch (tag) {
        case kAnchor:
            return "Anchor";
        case kBacklashCompensation:
            return "Backlash Compensation";
        case kBridge:
            return "Bridge";
        case kConnection:
            return "Connection";
        case kFlickMove:
            return "Flick Move";
        case kInfill:
            return "Infill";
        case kInset:
            return "Inset";
        case kOutline:
            return "Outline";
        case kLeakyTravelMove:
            return "Leaky Travel Move";
        case kTravelMove:
            return "Travel Move";
        case kTrailingExtrusionMove:
            return "Trailing Extrusion Move";
        case kSpur:
            return "Spur";
        case kSupport:
            return "Support";
        case kRetract:
            return "Retract";
        case kRestart:
            return "Restart";
        case kFlick:
            return "Flick";
        case kPurge:
            return "Purge";
        case kLongRetract:
            return "Long Retract";
        case kLongRestart:
            return "Long Restart";
        case kBeadModeExternal:
          return "BeadMode External";
        case kBeadModeExternalThick:
          return "BeadMode External Thick";
        case kBeadModeExternalThicker:
          return "BeadMode External Thicker";
        case kBeadModeExternalThickest:
          return "BeadMode External Thickest";
        case kBeadModeInternal:
          return "BeadMode Internal";
        case kBeadModeInternalThick:
          return "BeadMode Internal Thick";
        case kBeadModeInternalThicker:
          return "BeadMode Internal Thicker";
        case kBeadModeInternalThickest:
          return "BeadMode Internal Thickest";
        case kBeadModeUser1:
          return "BeadMode User1";
        case kBeadModeUser2:
          return "BeadMode User2";
        case kBeadModeUser3:
          return "BeadMode User3";
        case kBeadModeUser4:
          return "BeadMode User4";
        case LAST:
        default:
            return "";
    }
}

inline int stringToToolpathTag(const std::string &str) {
    for(unsigned i = 0; i < LAST; i++) {
        if (str == stringFromToolpathTag((ToolpathTag)i)) {
            return i;
        }
    }
    return -1;
}

enum AxisUnit {
    kMilliMetre=0,
    kMicroMetre,
    kMilliLitre,
    kMicroLitre,
    kLast
};

inline std::string stringFromAxisUnit(const AxisUnit unit) {
    switch(unit) {
    case kMilliMetre:
        return "mm";
    case kMicroMetre:
        return "um";
    case kMilliLitre:
        return "ml";
    case kMicroLitre:
        return "ul";
    case kLast:
    default:
        return "";
    }
}

inline int stringToAxisUnit(const std::string& str) {
    for(int i = 0; i < kLast; i++) {
        if (str == stringFromAxisUnit((AxisUnit)i)) {
            return i;
        }
    }
    return -1;
}
}
   
#endif // INCLUDE_MBCOREUTILS_TOOLPATH_TAGS_H_
