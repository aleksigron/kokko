# Entity-Component-System design

Entities are just IDs, that are used to connect related components into an entity. Entities do not know what components they have, you can only query if an entity has a component of a certain type from the system that manages those components.

Entities are managed by EntityManager. It has a very simple interface; you can only create and destroy entities and ask if an entity is alive.

When you want to attach a component to an entity, you ask the system that manages the component type to create a component for that entity. It is the responsibility of each system how it manages the connection of its components to entities. Some systems might allow multiple components to be added to a single entity, some might only let one component be attached to each entity.