TARGET := numptyphysics

NUMPTY_DIR := $(call select_from_ports,numptyphysics)/src/app/numptyphysics

SRC_CC := $(filter-out Swipe.cpp,$(notdir $(wildcard $(NUMPTY_DIR)/*.cpp)))

SRC_CC += Box2D/Source/Dynamics/b2Body.cpp \
          Box2D/Source/Dynamics/b2Island.cpp \
          Box2D/Source/Dynamics/b2World.cpp \
          Box2D/Source/Dynamics/b2ContactManager.cpp \
          Box2D/Source/Dynamics/Contacts/b2Contact.cpp \
          Box2D/Source/Dynamics/Contacts/b2PolyContact.cpp \
          Box2D/Source/Dynamics/Contacts/b2CircleContact.cpp \
          Box2D/Source/Dynamics/Contacts/b2PolyAndCircleContact.cpp \
          Box2D/Source/Dynamics/Contacts/b2ContactSolver.cpp \
          Box2D/Source/Dynamics/b2WorldCallbacks.cpp \
          Box2D/Source/Dynamics/Joints/b2MouseJoint.cpp \
          Box2D/Source/Dynamics/Joints/b2PulleyJoint.cpp \
          Box2D/Source/Dynamics/Joints/b2Joint.cpp \
          Box2D/Source/Dynamics/Joints/b2RevoluteJoint.cpp \
          Box2D/Source/Dynamics/Joints/b2PrismaticJoint.cpp \
          Box2D/Source/Dynamics/Joints/b2DistanceJoint.cpp \
          Box2D/Source/Dynamics/Joints/b2GearJoint.cpp \
          Box2D/Source/Common/b2StackAllocator.cpp \
          Box2D/Source/Common/b2Math.cpp \
          Box2D/Source/Common/b2BlockAllocator.cpp \
          Box2D/Source/Common/b2Settings.cpp \
          Box2D/Source/Collision/b2Collision.cpp \
          Box2D/Source/Collision/b2Distance.cpp \
          Box2D/Source/Collision/Shapes/b2Shape.cpp \
          Box2D/Source/Collision/Shapes/b2CircleShape.cpp \
          Box2D/Source/Collision/Shapes/b2PolygonShape.cpp \
          Box2D/Source/Collision/b2TimeOfImpact.cpp \
          Box2D/Source/Collision/b2PairManager.cpp \
          Box2D/Source/Collision/b2CollidePoly.cpp \
          Box2D/Source/Collision/b2CollideCircle.cpp \
          Box2D/Source/Collision/b2BroadPhase.cpp

SRC_CC += os/OsFreeDesktop.cpp

vpath %.cpp $(NUMPTY_DIR)

Dialogs.o: help_text_html.h
help_text_html.h: help_text.html
	$(VERBOSE)(cd $(NUMPTY_DIR); xxd -i help_text.html) > $@

vpath help_text.html $(NUMPTY_DIR)

SRC_CC += dummy.cc
vpath dummy.cc $(PRG_DIR)

SRC_CC += getenv.cc
vpath getenv.cc $(PRG_DIR)

INC_DIR += $(NUMPTY_DIR) $(NUMPTY_DIR)/Box2D/Include

LIBS += base posix stdcxx
LIBS += sdl sdl_image sdl_ttf zlib

CC_OPT_Canvas := -DGENODE

$(TARGET): numptyphysics_data.tar
numptyphysics_data.tar:
	$(VERBOSE)cd $(NUMPTY_DIR)/data; tar cf $(PWD)/bin/$@ .

CC_OPT += -DINSTALL_BASE_PATH='"/"' -DUSER_BASE_PATH='"/"'

