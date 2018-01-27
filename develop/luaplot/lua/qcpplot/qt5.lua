-- qt5.lua

-- 定义了本模块
local modname = ...
local M = {
    QWIDGETSIZE_MAX=(2^24 - 1), -- ((1<<24)-1)

    -- enum ConnectionType {
    AutoConnection=0,
    DirectConnection=1,
    QueuedConnection=2,
    BlockingQueuedConnection=3,
    UniqueConnection = 0x80
    ,
--  };

    -- enum GlobalColor {
    color0=0, color1=1, black=2, white=3, darkGray=4, gray=5, lightGray=6, red=7, green=8,
    blue=9, cyan=10, magenta=11, yellow=12, darkRed=13, darkGreen=14, darkBlue=15, darkCyan=16,
    darkMagenta=17, darkYellow=18, transparent=19
    ,
--  }

    --enum PenStyle { // pen style
    NoPen=0,
    SolidLine=1,
    DashLine=2,
    DotLine=3,
    DashDotLine=4,
    DashDotDotLine=5,
    CustomDashLine=6
--#ifndef Q_MOC_RUN
    , MPenStyle = 0x0f
    ,
--#endif
--  };

    -- enum BrushStyle { // brush style
    NoBrush=0,
    SolidPattern=1,
    Dense1Pattern=2,
    Dense2Pattern=3,
    Dense3Pattern=4,
    Dense4Pattern=5,
    Dense5Pattern=6,
    Dense6Pattern=7,
    Dense7Pattern=8,
    HorPattern=9,
    VerPattern=10,
    CrossPattern=11,
    BDiagPattern=12,
    FDiagPattern=13,
    DiagCrossPattern=14,
    LinearGradientPattern=15,
    RadialGradientPattern=16,
    ConicalGradientPattern=17,
    TexturePattern = 24
    ,
--  };

    -- enum TimeSpec {
    LocalTime=0,
    UTC=1,
    OffsetFromUTC=2,
    TimeZone=3
    ,
--  };

    -- enum DayOfWeek {
    Monday = 1,
    Tuesday = 2,
    Wednesday = 3,
    Thursday = 4,
    Friday = 5,
    Saturday = 6,
    Sunday = 7
    ,
--  };

    -- enum WidgetAttribute {
    WA_Disabled = 0,
    WA_UnderMouse = 1,
    WA_MouseTracking = 2,
    WA_ContentsPropagated = 3, -- ## deprecated
    WA_OpaquePaintEvent = 4,
    WA_NoBackground = WA_OpaquePaintEvent, -- ## deprecated
    WA_StaticContents = 5,
    WA_LaidOut = 7,
    WA_PaintOnScreen = 8,
    WA_NoSystemBackground = 9,
    WA_UpdatesDisabled = 10,
    WA_Mapped = 11,
    WA_MacNoClickThrough = 12, -- Mac only
    WA_InputMethodEnabled = 14,
    WA_WState_Visible = 15,
    WA_WState_Hidden = 16,

    WA_ForceDisabled = 32,
    WA_KeyCompression = 33,
    WA_PendingMoveEvent = 34,
    WA_PendingResizeEvent = 35,
    WA_SetPalette = 36,
    WA_SetFont = 37,
    WA_SetCursor = 38,
    WA_NoChildEventsFromChildren = 39,
    WA_WindowModified = 41,
    WA_Resized = 42,
    WA_Moved = 43,
    WA_PendingUpdate = 44,
    WA_InvalidSize = 45,
    WA_MacBrushedMetal = 46, -- Mac only
    WA_MacMetalStyle = WA_MacBrushedMetal, -- obsolete
    WA_CustomWhatsThis = 47,
    WA_LayoutOnEntireRect = 48,
    WA_OutsideWSRange = 49,
    WA_GrabbedShortcut = 50,
    WA_TransparentForMouseEvents = 51,
    WA_PaintUnclipped = 52,
    WA_SetWindowIcon = 53,
    WA_NoMouseReplay = 54,
    WA_DeleteOnClose = 55,
    WA_RightToLeft = 56,
    WA_SetLayoutDirection = 57,
    WA_NoChildEventsForParent = 58,
    WA_ForceUpdatesDisabled = 59,

    WA_WState_Created = 60,
    WA_WState_CompressKeys = 61,
    WA_WState_InPaintEvent = 62,
    WA_WState_Reparented = 63,
    WA_WState_ConfigPending = 64,
    WA_WState_Polished = 66,
    WA_WState_DND = 67, -- ## deprecated
    WA_WState_OwnSizePolicy = 68,
    WA_WState_ExplicitShowHide = 69,

    WA_ShowModal = 70, -- ## deprecated
    WA_MouseNoMask = 71,
    WA_GroupLeader = 72, -- ## deprecated
    WA_NoMousePropagation = 73, -- ## for now, might go away.
    WA_Hover = 74,
    WA_InputMethodTransparent = 75, -- Don't reset IM when user clicks on this (for virtual keyboards on embedded)
    WA_QuitOnClose = 76,

    WA_KeyboardFocusChange = 77,

    WA_AcceptDrops = 78,
    WA_DropSiteRegistered = 79, -- internal
    WA_ForceAcceptDrops = WA_DropSiteRegistered, -- ## deprecated

    WA_WindowPropagation = 80,

    WA_NoX11EventCompression = 81,
    WA_TintedBackground = 82,
    WA_X11OpenGLOverlay = 83,
    WA_AlwaysShowToolTips = 84,
    WA_MacOpaqueSizeGrip = 85,
    WA_SetStyle = 86,

    WA_SetLocale = 87,
    WA_MacShowFocusRect = 88,

    WA_MacNormalSize = 89,  -- Mac only
    WA_MacSmallSize = 90,   -- Mac only
    WA_MacMiniSize = 91,    -- Mac only

    WA_LayoutUsesWidgetRect = 92,
    WA_StyledBackground = 93, -- internal
    WA_MSWindowsUseDirect3D = 94, -- Win only
    WA_CanHostQMdiSubWindowTitleBar = 95, -- Internal

    WA_MacAlwaysShowToolWindow = 96, -- Mac only

    WA_StyleSheet = 97, -- internal

    WA_ShowWithoutActivating = 98,

    WA_X11BypassTransientForHint = 99,

    WA_NativeWindow = 100,
    WA_DontCreateNativeAncestors = 101,

    WA_MacVariableSize = 102,    -- Mac only

    WA_DontShowOnScreen = 103,

    -- window types from http:--standards.freedesktop.org/wm-spec/
    WA_X11NetWmWindowTypeDesktop = 104,
    WA_X11NetWmWindowTypeDock = 105,
    WA_X11NetWmWindowTypeToolBar = 106,
    WA_X11NetWmWindowTypeMenu = 107,
    WA_X11NetWmWindowTypeUtility = 108,
    WA_X11NetWmWindowTypeSplash = 109,
    WA_X11NetWmWindowTypeDialog = 110,
    WA_X11NetWmWindowTypeDropDownMenu = 111,
    WA_X11NetWmWindowTypePopupMenu = 112,
    WA_X11NetWmWindowTypeToolTip = 113,
    WA_X11NetWmWindowTypeNotification = 114,
    WA_X11NetWmWindowTypeCombo = 115,
    WA_X11NetWmWindowTypeDND = 116,

    WA_MacFrameworkScaled  = 117,

    WA_SetWindowModality = 118,
    WA_WState_WindowOpacitySet = 119, -- internal
    WA_TranslucentBackground = 120,

    WA_AcceptTouchEvents = 121,
    WA_WState_AcceptedTouchBeginEvent = 122,
    WA_TouchPadAcceptSingleTouchEvents = 123,

    WA_X11DoNotAcceptFocus = 126,
    WA_MacNoShadow = 127,

    WA_AlwaysStackOnTop = 128,

    WA_TabletTracking = 129,

    -- Add new attributes before this line
    WA_AttributeCount
    ,
--  };

    -- enum AlignmentFlag {
    AlignLeft = 0x0001,
    AlignLeading = AlignLeft,
    AlignRight = 0x0002,
    AlignTrailing = AlignRight,
    AlignHCenter = 0x0004,
    AlignJustify = 0x0008,
    AlignAbsolute = 0x0010,
    --AlignHorizontal_Mask = AlignLeft + AlignRight + AlignHCenter + AlignJustify + AlignAbsolute,

    AlignTop = 0x0020,
    AlignBottom = 0x0040,
    AlignVCenter = 0x0080,
    AlignBaseline = 0x0100,
    -- Note that 0x100 will clash with Qt::TextSingleLine = 0x100 due to what the comment above
    -- this enum declaration states. However, since Qt::AlignBaseline is only used by layouts,
    -- it doesn't make sense to pass Qt::AlignBaseline to QPainter::drawText(), so there
    -- shouldn't really be any ambiguity between the two overlapping enum values.
    --AlignVertical_Mask = AlignTop + AlignBottom + AlignVCenter + AlignBaseline,

    --AlignCenter = AlignVCenter + AlignHCenter
--  };
--  Q_DECLARE_FLAGS(Alignment, AlignmentFlag)


  }

-- enum AlignmentFlag {
M.AlignHorizontal_Mask = M.AlignLeft + M.AlignRight + M.AlignHCenter + M.AlignJustify + M.AlignAbsolute
M.AlignVertical_Mask = M.AlignTop + M.AlignBottom + M.AlignVCenter + M.AlignBaseline
M.AlignCenter = M.AlignVCenter + M.AlignHCenter

M.xxx = {

  }

-- return modname的功能
_G[modname]=M
package.loaded[modname] = M  

-- 返回本模块的table
return M

