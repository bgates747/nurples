import math
import matplotlib.pyplot as plt

def polar_to_cartesian(magnitude, angle_deg):
    """Converts polar coordinates to Cartesian coordinates."""
    angle_rad = math.radians(angle_deg)
    # sin and cos have been switched and cos negated from the usual formulae
    # the result is that zero degrees points north and angles increase clockwise
    # i.e. navigational conventions, which i find more intuitive for 2d games
    x = magnitude * math.sin(angle_rad)
    y = magnitude * -math.cos(angle_rad)
    return (x, y)

def solve_quadratic(a, b, c):
    """Solves the quadratic equation ax^2 + bx + c = 0"""
    discriminant = b**2 - 4*a*c
    
    if discriminant < 0:
        return None, None  # No real solutions
    elif discriminant == 0:
        t = -b / (2 * a)
        return t, None  # Only one solution
    else:
        sqrt_discriminant = math.sqrt(discriminant)
        t1 = (-b + sqrt_discriminant) / (2 * a)
        t2 = (-b - sqrt_discriminant) / (2 * a)
        return t1, t2  # Two solutions

def closest_approach_time(P1, V1, P2, V2, r1, r2):
    """
    P1, P2: Initial positions of the circles (tuples of x, y)
    V1, V2: Velocities of the circles (tuples of vx, vy)
    r1, r2: Radii of the circles
    Returns the time(s) of closest approach or collision.
    """
    # Relative position and velocity
    Rx, Ry = P2[0] - P1[0], P2[1] - P1[1]
    Vrel_x, Vrel_y = V2[0] - V1[0], V2[1] - V1[1]

    # Coefficients of the quadratic equation
    a = Vrel_x**2 + Vrel_y**2
    b = 2 * (Rx * Vrel_x + Ry * Vrel_y)
    c = Rx**2 + Ry**2 - (r1 + r2)**2

    # Solve the quadratic equation
    t1, t2 = solve_quadratic(a, b, c)

    return t1, t2

def circle_position(P, V, t):
    """Calculate position of a circle at time t."""
    return (P[0] + V[0] * t, P[1] + V[1] * t)


def do_collision_simulation(P1, V1_polar, P2, V2_polar, r1, r2, te):
    # Convert polar velocities to Cartesian coordinates
    V1 = polar_to_cartesian(V1_polar[0], V1_polar[1])
    V2 = polar_to_cartesian(V2_polar[0], V2_polar[1])

    t1, t2 = closest_approach_time(P1, V1, P2, V2, r1, r2)

    # Check if there is a valid closest approach time
    if t1 is None and t2 is None:
        # No valid collision time
        t_closest = None
    elif t1 is not None and t2 is not None:
        # Both t1 and t2 are defined and non-negative
        if t1 >= 0 and t2 >= 0:
            t_closest = min(t1, t2)  # Choose the earliest valid time
        elif t1 >= 0:
            t_closest = t1
        elif t2 >= 0:
            t_closest = t2
        else:
            t_closest = None
    elif t1 is not None and t1 >= 0:
        t_closest = t1
    elif t2 is not None and t2 >= 0:
        t_closest = t2
    else:
        t_closest = None

    # Calculate positions at start and end times
    pos1_start = P1
    pos2_start = P2
    pos1_end = circle_position(P1, V1, te)
    pos2_end = circle_position(P2, V2, te)

    # Plotting
    fig, ax = plt.subplots()

    # Plot start positions with filled circles
    circle1_start = plt.Circle(pos1_start, r1, color='blue', fill=True, alpha=0.5)
    circle2_start = plt.Circle(pos2_start, r2, color='red', fill=True, alpha=0.5)
    ax.add_patch(circle1_start)
    ax.add_patch(circle2_start)

    # Only plot closest approach if it exists
    if t_closest is not None:
        pos1_closest = circle_position(P1, V1, t_closest)
        pos2_closest = circle_position(P2, V2, t_closest)

        # Plot closest approach positions with unfilled circles
        circle1_closest = plt.Circle(pos1_closest, r1, color='blue', fill=False, linestyle='--')
        circle2_closest = plt.Circle(pos2_closest, r2, color='red', fill=False, linestyle='--')
        ax.add_patch(circle1_closest)
        ax.add_patch(circle2_closest)

        # Draw lines with arrowheads connecting the centers at each stage
        ax.plot([pos1_start[0], pos1_closest[0], pos1_end[0]], [pos1_start[1], pos1_closest[1], pos1_end[1]], 'k-')
        ax.plot([pos2_start[0], pos2_closest[0], pos2_end[0]], [pos2_start[1], pos2_closest[1], pos2_end[1]], 'k-')

        # Add arrowheads to the lines indicating direction of motion
        ax.annotate('', xy=pos1_closest, xytext=pos1_start, arrowprops=dict(arrowstyle="->", color='black'))
        ax.annotate('', xy=pos1_end, xytext=pos1_closest, arrowprops=dict(arrowstyle="->", color='black'))
        ax.annotate('', xy=pos2_closest, xytext=pos2_start, arrowprops=dict(arrowstyle="->", color='black'))
        ax.annotate('', xy=pos2_end, xytext=pos2_closest, arrowprops=dict(arrowstyle="->", color='black'))
    else:
        # Draw lines with arrowheads directly from start to end
        ax.plot([pos1_start[0], pos1_end[0]], [pos1_start[1], pos1_end[1]], 'k-')
        ax.plot([pos2_start[0], pos2_end[0]], [pos2_start[1], pos2_end[1]], 'k-')
        
        ax.annotate('', xy=pos1_end, xytext=pos1_start, arrowprops=dict(arrowstyle="->", color='black'))
        ax.annotate('', xy=pos2_end, xytext=pos2_start, arrowprops=dict(arrowstyle="->", color='black'))

    # Plot end positions with filled circles
    circle1_end = plt.Circle(pos1_end, r1, color='blue', fill=True, alpha=0.5)
    circle2_end = plt.Circle(pos2_end, r2, color='red', fill=True, alpha=0.5)
    ax.add_patch(circle1_end)
    ax.add_patch(circle2_end)

    # Set the major ticks for each axis at intervals of 8
    ticks = 64
    xmin = 0 ; xmax = 320
    ymin = 0 ; ymax = 240
    ax.set_xticks(range(xmin, xmax+1, ticks))
    ax.set_yticks(range(ymin, ymax+1, ticks))

    # Set plot limits and labels
    ax.set_xlim(xmin, xmax)
    ax.set_ylim(ymin, ymax)
    ax.set_aspect('equal', 'box')
    ax.set_title("Circles' Positions: Start, Closest Approach, and End")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")

    # Invert the y-axis
    ax.invert_yaxis()

    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    # Initial positions in Cartesian coordinates
    P1 = (160, 200)  # player sprite
    P2 = (180, 160)  # enemy sprite

    # Velocities in polar coordinates (magnitude, angle in degrees)
    V1_polar = (3, 315)
    V2_polar = (2.5, 270)

    # Radii of the circles
    r1 = 8  # Radius of player sprite
    r2 = 8  # Radius of enemy sprite

    te = 30  # elapsed time between start and end positions

    do_collision_simulation(P1, V1_polar, P2, V2_polar, r1, r2, te)
