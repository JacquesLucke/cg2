import numpy as np
import matplotlib.pyplot as plt
import scipy.interpolate as si
import sys

def draw_splines(filename, controlpoints, degree = 1):
    plt.plot(controlpoints[:,0], controlpoints[:,1], "o", label="Kontrollpunkte", color="black")
    p = scipy_bspline(controlpoints, 100, degree);
    x, y = p.T
    plt.plot(x, y, "k-", label="B-Spline", color="blue")
    plt.legend()
    plt.xlabel("x")
    plt.ylabel("y")
    plt.xlim(-1, 5)
    plt.ylim(-1, 4)
    plt.gca().set_aspect("equal", adjustable="box")
    plt.savefig(filename + ".png", transparent = True)
    

def scipy_bspline(cv, n=100, degree=3):
    """ Calculate n samples on a bspline

        cv :      Array ov control vertices
        n  :      Number of samples to return
        degree:   Curve degree
    """
    cv = np.asarray(cv)
    count = cv.shape[0]

    degree = np.clip(degree,1,count-1)
    kv = np.clip(np.arange(count+degree+1)-degree,0,count-degree)

    # Return samples
    spl = si.BSpline(kv, cv, degree)
    return spl(np.linspace(0,count - degree,n))

 
    
if __name__ == "__main__":
    filename = "splines"
    if len(sys.argv) >= 2: filename = sys.argv[1]
    draw_splines(filename, np.array([[0, 0], [1, 3], [2, 1], [3, 2], [4, 1]]))

   
