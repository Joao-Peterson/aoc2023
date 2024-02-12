pkg load symbolic;

###### part 1

syms x y t x1 y1 vx1 vy1 x2 y2 vx2 vy2 z1 z2 vz1 vz2;

x == x1 + vx1 * t;
e = solve(ans, t);
y == y1 + vy1 * t;
e == solve(ans, t);
ey = solve(ans, y);

% fy = function_handle(ey, 'vars', [x1, y1, vx1, vy1]);

subs(ey, x1, x2);
subs(ans, y1, y2);
subs(ans, vx1, vx2);
subs(ans, vy1, vy2);
ey == ans;
ex = solve(ans, x);

ccode(ey)
ccode(ex)

###### part 2

# make x coord of two trails the same
x1 + vx1 * t == x2 + vx2 * t;
# solve for t
ex = solve(ans, t);

# make a y copy
ey = ex;
ey = subs(ey, x1, y1);
ey = subs(ey, x2, y2);
ey = subs(ey, vx1, vy1);
ey = subs(ey, vx2, vy2);

# make a z copy
ez = ex;
ez = subs(ex, x1, z1);
ez = subs(ex, x2, z2);
ez = subs(ex, vx1, vz1);
ez = subs(ex, vx2, vz2);

# did this by hand :P
f1 = (-x1 + x2) * (vy1 - vy2) == (-y1 + y2) * (vx1 - vx2)
f2 = (-y1 + y2) * (vz1 - vz2) == (-z1 + z2) * (vy1 - vy2)

eqs = {};

x1 = 19;
y1 = 13;
z1 = 30;
vx1 = -2;
vy1 = 1;
vz1 = -2;
eqs{1} = eval(f1);
eqs{2} = eval(f2);

x1 = 19;
y1 = 13;
z1 = 30;
vx1 = -2;
vy1 =  1;
vz1 =  -2;
eqs{3} = eval(f1);
eqs{4} = eval(f2);

x1 =  18;
y1 =  19;
z1 =  22;
vx1 =  -1;
vy1 =  -1;
vz1 =  -2;
eqs{5} = eval(f1);
eqs{6} = eval(f2);

x1 =  20;
y1 =  25;
z1 =  34;
vx1 =  -2;
vy1 =  -2;
vz1 =  -4;
eqs{7} = eval(f1);
eqs{8} = eval(f2);

x1 =  12;
y1 =  31;
z1 =  28;
 vx1 = -1;
 vy1 = -2;
 vz1 = -1;
eqs{9} = eval(f1);
eqs{10} = eval(f2);

x1 =  20;
y1 =  19;
z1 =  15;
 vx1 =  1;
 vy1 =  -5;
 vz1 =  -3;
eqs{11} = eval(f1);
eqs{12} = eval(f2);

% eqs{:}
# solving using symbolic pkg solver 
s = solve(eqs{:});

s.x2
s.y2
s.z2
s.vx2
s.vy2
s.vz2

###### part 2 but awesome!

syms x y t x1 y1 vx1 vy1 x2 y2 vx2 vy2 z1 z2 vz1 vz2 xr yr zr vxr vyr vzr; 

# by hand time equality for two trails
f1 = (-x1 + x2) * (vy1 - vy2) == (-y1 + y2) * (vx1 - vx2)

# time equality for trail '1' and rock trail 'r', in 'x, y'
xy1 = f1;
xy1 = subs(xy1, x2, xr);
xy1 = subs(xy1, y2, yr);
xy1 = subs(xy1, vx2, vxr);
xy1 = subs(xy1, vy2, vyr);

# time equality for trail '1' and rock trail 'r', in 'x, z'
xz1 = xy1;
xz1 = subs(xz1, y1, z1);
xz1 = subs(xz1, yr, zr);
xz1 = subs(xz1, vy1, vz1);
xz1 = subs(xz1, vyr, vzr);

# time equality for trail '2' and rock trail 'r', in 'x, y'
xy2 = xy1;
xy2 = subs(xy2, x1, x2);
xy2 = subs(xy2, y1, y2);
xy2 = subs(xy2, vx1, vx2);
xy2 = subs(xy2, vy1, vy2);

# time equality for trail '2' and rock trail 'r', in 'x, z'
xz2 = xy2;
xz2 = subs(xz2, y2, z2);
xz2 = subs(xz2, yr, zr);
xz2 = subs(xz2, vy2, vz2);
xz2 = subs(xz2, vyr, vzr);

# substitute to eliminate nonlinear terms of 'r', by substracting the equality from trail 1 and trail 2
# xy1 - xy2
m1 = expand(lhs(xy1) - rhs(xy1)) - expand(lhs(xy2) - rhs(xy2)) == 0;
# xz1 - xz2
m2 = expand(lhs(xz1) - rhs(xz1)) - expand(lhs(xz2) - rhs(xz2)) == 0;

# get matrix form of the two linear equations in realation to the 'r' variables: xr, yr, zr, vxr, vyr, vzr 
[A, b] = equationsToMatrix(m1, m2, xr, yr, zr, vxr, vyr, vzr);

# get C code, yeahhh!
ccode(A)
ccode(b)

# test
A = [
	[-10,              234,                0,  -23876145662582,   64433973011902,                0],
    [-350,                0,              234, -120910462712744,                0,   64433973011902],
    [  30,               48,                0,  -35087306144574, -120482468054108,                0],
    [-241,                0,               48,  -17459646842055,                0, -120482468054108],
    [  71,              119,                0,   12658037039421,   -5502732716787,                0],
    [-223,                0,              119,  -48818292032493,                0,   -5502732716787],
];

b = transpose([66600821482191620, -22899742088591160, 23909515091138932, -38929699753334554, 55916761531344922, -17702593832787840]);

linsolve(A, b)