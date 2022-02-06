import numpy as np
from spatialmath import SE3
import with_respect_to as WRT

db = WRT.DbConnector()

pose = np.array([[1,0,0,1],[0,1,0,1],[0,0,1,1],[0,0,0,1]])
db.In('test').Set('a').Wrt('world').Ei('world').As(pose)

pose = SE3.Rx(90, "deg").A
db.In('test').Set('b').Wrt('a').Ei('a').As(pose)

pose = SE3.Rx(0, "deg", t=[1,0,0]).A
db.In('test').Set('c').Wrt('b').Ei('b').As(pose)

pose = SE3.Rz(90, "deg", t=[1,1,0]).A
db.In('test').Set('d').Wrt('b').Ei('b').As(pose)


assert(SE3(db.In('test').Get('a').Wrt('b').Ei('b'))          == SE3(np.array([[1,0,0,0],[0,0,1,0],[0,-1,0,0],[0,0,0,1]])))
assert(SE3(db.In('test').Get('a').Wrt('b').Ei('a'))          == SE3(np.eye(4)))
assert(SE3(db.In('test').Get('c').Wrt('world').Ei('world'))  == SE3(np.array([[1,0,0,2],[0,0,-1,1],[0,1,0,1],[0,0,0,1]])))
assert(SE3(db.In('test').Get('c').Wrt('world').Ei('c'))      == SE3(np.array([[1,0,0,2],[0,1,0,1],[0,0,1,-1],[0,0,0,1]])))
assert(SE3(db.In('test').Get('c').Wrt('world').Ei('a'))      == SE3(np.array([[1,0,0,2],[0,0,-1,1],[0,1,0,1],[0,0,0,1]])))
assert(SE3(db.In('test').Get('d').Wrt('a').Ei('a'))          == SE3(np.array([[0,-1,0,1],[0,0,-1,0],[1,0,0,1],[0,0,0,1]])))

print("All tests passed!")

