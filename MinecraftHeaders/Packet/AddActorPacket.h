#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "../Core/Packet.h"
#include "../Core/DataItem.h"
#include "../Math/Vec3.h"
#include "../Math/Vec2.h"
#include "../Actor/ActorDefinitionIdentifier.h"
#include "../Actor/AttributeInstanceHandle.h"
#include "../Actor/ActorLink.h"
#include "../Actor/SyncedAttribute.h"
#include "../Actor/ActorUniqueID.h"
#include "../Actor/ActorRuntimeID.h"
#include "../dll.h"

class AddActorPacket : public Packet {
public:
  std::vector<ActorLink> links;
  ActorUniqueID uid;
  ActorRuntimeID rid;
  uint64_t type;
  std::vector<std::unique_ptr<DataItem>> items;
  ActorDefinitionIdentifier act_id;
  Vec3 pos, speed;
  Vec2 rot;
  float headYaw;
  std::vector<AttributeInstanceHandle> attributes;
  std::vector<SyncedAttribute> synced;

  inline ~AddActorPacket() {}
  MCAPI virtual MinecraftPacketIds getId() const;
  MCAPI virtual std::string getName() const;
  MCAPI virtual void write(BinaryStream &) const;
  MCAPI virtual PacketReadResult read(ReadOnlyBinaryStream &);
};

static_assert(offsetof(AddActorPacket, act_id) == 112);
static_assert(offsetof(AddActorPacket, rot) == 304);
static_assert(offsetof(AddActorPacket, attributes) == 320);
static_assert(offsetof(AddActorPacket, synced) == 344);