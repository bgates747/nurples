import bpy
import math

# Function to clear existing mesh objects and materials
def clear_scene():
    # Delete all mesh objects
    bpy.ops.object.select_all(action='DESELECT')
    bpy.ops.object.select_by_type(type='MESH')
    bpy.ops.object.delete()

    # Remove all materials
    for material in bpy.data.materials:
        bpy.data.materials.remove(material)

# Function to apply the default material to a mesh object
def apply_default_material(obj):
    if len(bpy.data.materials) == 0:
        # Create a new material if none exist
        mat = bpy.data.materials.new(name="DefaultMaterial")
    else:
        # Use the first material in the list (default material)
        mat = bpy.data.materials[0]
    
    if obj.data.materials:
        obj.data.materials[0] = mat
    else:
        obj.data.materials.append(mat)

# Clear the scene
clear_scene()

# Create the fuselage
bpy.ops.mesh.primitive_cube_add(size=2, location=(0, 0, 0))
fuselage = bpy.context.object
fuselage.scale = (1, 0.3, 0.3)
fuselage.name = "Fuselage"
apply_default_material(fuselage)

# Create the cockpit
bpy.ops.mesh.primitive_uv_sphere_add(segments=32, ring_count=16, radius=0.3, location=(0.7, 0, 0.3))
cockpit = bpy.context.object
cockpit.name = "Cockpit"
apply_default_material(cockpit)

# Create wings
def create_wing(position):
    bpy.ops.mesh.primitive_cube_add(size=1, location=position)
    wing = bpy.context.object
    wing.scale = (2.5, 0.1, 0.5)
    wing.name = "Wing"
    apply_default_material(wing)
    return wing

wing1 = create_wing((1.5, 1.5, 0))
wing2 = create_wing((1.5, -1.5, 0))
wing3 = create_wing((-1.5, 1.5, 0))
wing4 = create_wing((-1.5, -1.5, 0))

# Create engines
def create_engine(position):
    bpy.ops.mesh.primitive_cylinder_add(radius=0.2, depth=1.5, location=position)
    engine = bpy.context.object
    engine.rotation_euler = (math.radians(90), 0, 0)  # Align the cylinder along the x-axis
    engine.name = "Engine"
    apply_default_material(engine)
    return engine

engine1 = create_engine((2.5, 1.5, 0))
engine2 = create_engine((2.5, -1.5, 0))
engine3 = create_engine((-2.5, 1.5, 0))
engine4 = create_engine((-2.5, -1.5, 0))

# Create laser cannons
def create_laser_cannon(position):
    bpy.ops.mesh.primitive_cylinder_add(radius=0.05, depth=2, location=position)
    cannon = bpy.context.object
    cannon.rotation_euler = (math.radians(90), 0, 0)  # Align the cylinder along the x-axis
    cannon.name = "Laser_Cannon"
    apply_default_material(cannon)
    return cannon

cannon1 = create_laser_cannon((3.5, 1.5, 0.3))
cannon2 = create_laser_cannon((3.5, -1.5, 0.3))
cannon3 = create_laser_cannon((-3.5, 1.5, 0.3))
cannon4 = create_laser_cannon((-3.5, -1.5, 0.3))

# Join all components into a single object
bpy.ops.object.select_all(action='DESELECT')
fuselage.select_set(True)
cockpit.select_set(True)
wing1.select_set(True)
wing2.select_set(True)
wing3.select_set(True)
wing4.select_set(True)
engine1.select_set(True)
engine2.select_set(True)
engine3.select_set(True)
engine4.select_set(True)
cannon1.select_set(True)
cannon2.select_set(True)
cannon3.select_set(True)
cannon4.select_set(True)
bpy.context.view_layer.objects.active = fuselage
bpy.ops.object.join()  # Combine all parts into one mesh
