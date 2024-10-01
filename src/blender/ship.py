import bpy
import math
from mathutils import Vector

# Function to clear existing mesh objects and materials
def clear_scene():
    # Delete all mesh objects
    bpy.ops.object.select_all(action='DESELECT')
    bpy.ops.object.select_by_type(type='MESH')
    bpy.ops.object.delete()

    # Remove all materials
    for material in bpy.data.materials:
        bpy.data.materials.remove(material)

# Function to create an ellipsoidal fuselage
def create_fuselage():
    bpy.ops.mesh.primitive_uv_sphere_add(segments=32, ring_count=16, radius=1, location=(0, 0, 0))
    fuselage = bpy.context.object
    fuselage.scale = (2, 1, 1)  # Scale to make it ellipsoidal (squashed cigar)
    fuselage.name = "Fuselage"
    return fuselage

# Function to create delta wings
def create_wing():
    bpy.ops.mesh.primitive_plane_add(size=2, location=(0, 0, 0))
    wing = bpy.context.object
    wing.scale = (4, 1.5, 0.1)  # Scaling to get a delta wing shape
    wing.rotation_euler = (math.radians(0), math.radians(0), math.radians(30))  # Rotate to align with fuselage
    wing.location = (-0.5, -0.5, 0)  # Position relative to fuselage
    wing.name = "Wing"
    return wing

# Function to create twin vertical stabilizers
def create_vertical_stabilizer():
    bpy.ops.mesh.primitive_plane_add(size=1, location=(0, 0, 0))
    stabilizer = bpy.context.object
    stabilizer.scale = (1, 0.2, 0.1)  # Scaling to get a stabilizer shape
    stabilizer.rotation_euler = (math.radians(90), math.radians(0), math.radians(0))  # Rotate vertically
    stabilizer.name = "Vertical_Stabilizer"
    return stabilizer

# Function to create rocket engines
def create_engine():
    bpy.ops.mesh.primitive_cylinder_add(radius=0.3, depth=1, location=(0, 0, 0))
    engine = bpy.context.object
    engine.rotation_euler = (math.radians(90), 0, 0)  # Align the cylinder along the x-axis
    engine.name = "Engine"
    return engine

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

# Create fuselage
fuselage = create_fuselage()

# Create and position wings
left_wing = create_wing()
right_wing = left_wing.copy()
right_wing.data = left_wing.data.copy()
bpy.context.collection.objects.link(right_wing)
right_wing.location = (-0.5, 0.5, 0)
right_wing.rotation_euler = (math.radians(0), math.radians(0), math.radians(-30))

# Create and position vertical stabilizers
left_stabilizer = create_vertical_stabilizer()
left_stabilizer.location = (-1.8, -0.4, 0.7)
right_stabilizer = left_stabilizer.copy()
right_stabilizer.data = left_stabilizer.data.copy()
bpy.context.collection.objects.link(right_stabilizer)
right_stabilizer.location = (-1.8, 0.4, 0.7)

# Create and position engines
left_engine = create_engine()
left_engine.location = (-2.0, -0.4, 0)
right_engine = left_engine.copy()
right_engine.data = left_engine.data.copy()
bpy.context.collection.objects.link(right_engine)
right_engine.location = (-2.0, 0.4, 0)

# Apply default material to all parts
for obj in [fuselage, left_wing, right_wing, left_stabilizer, right_stabilizer, left_engine, right_engine]:
    apply_default_material(obj)

# Group all components
bpy.ops.object.select_all(action='DESELECT')
fuselage.select_set(True)
left_wing.select_set(True)
right_wing.select_set(True)
left_stabilizer.select_set(True)
right_stabilizer.select_set(True)
left_engine.select_set(True)
right_engine.select_set(True)
bpy.context.view_layer.objects.active = fuselage
bpy.ops.object.join()  # Combine all parts into one mesh
