#set math.mat(delim: "[")
#let pm = math.plus.minus
#let oplus = math.plus.circle
#let otimes = math.times.circle
#let ominus = math.minus.circle
#let subseteq = math.subset.eq
#let epsilon = math.epsilon.alt
#let sum = math.limits(math.sum)
#let lim = math.limits(math.lim)
#let max = math.limits(math.max)
#show link: underline
#show math.equation: it => {
  show sym.ast.op: sym.dot
  it
}

#let co = math.op("co")

== Graphics algorithms

Perspective matrix for horizontal fov $f in (0, pi)$, aspect ratio $a = w/h$, near plane $n > 0$ (assuming x is right, y is up,
z is forward) is given by

$ mat(1 / tan(f slash 2), 0, 0, 0; 0, a / ( tan(f slash 2)), 0, 0; 0, 0, 0, n; 0, 0, 1, 0) $ describing the function mapping a view space coordinate $(x, y, z, 1)$ to clip space:
$ (x, y, z, 1) |-> (x / tan(f slash 2), (a y) / ( tan(f slash 2)), n, z) $

According to #link("https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_rasterizer_desc")[rasterizer desc], in clip space, we clip to $-w <= x <= w, -w <= y <= w, 0 <= z <= w, w > 0$.
In terms of view-space coordinates, this is equivalent to clipping to

$ -z tan(f slash 2) <= x <= z tan(f slash 2) $
$ -(z tan(f slash 2)) / a <= y <= (z tan(f slash 2)) / a $
$ 0 <= n <= z $
$ z > 0 "(although this is already implied since n > 0)" $

Why can we clip in clip space? Because clipping is finding intersections with clipping planes using lines in clip space which correspond to lines in view space: let $p, q$ be two points in view space (padded with 1), and let $t$ make 
$(1-t)(P p) + t (P q)$ satisfy the the clip space inequalities. This is equal to $P((1-t)p + t q)$ where $(1-t)p + t q$ is a point in view space satisfying the view space inequalities.
// $ P^(-1) ((1-t)(P p) + t (P q)) = (1-t) p + t q $

Depth clip simply disables view $0 <= n <= z$, but $z > 0$ is still enforced. Note that if depth clip is disabled, then depth is *clamped to the viewport* before any depth testing takes place (see #link("https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-output-merger-stage")[docs]).

NDC is then given (in terms of view space) by
$ (x/(z tan(f slash 2)), (a y)/(z tan(f slash 2)), n/z) $


Now let $p_1, p_2, p_3 in RR^3$ be the vertices of a triangle in view space. Let $p = a p_1 + b p_2 + c p_3$ where $a, b, c >= 0, a + b + c = 1$ be a point inside the triangle. Let $f_1, f_2, f_3$ be attribute values at the vertices.

Evidently, the interpolated attribute value at $p$ is given by $F = a f_1 + b f_2 + c f_3$. But this is also equal to the value given by the rasterization formula (check rasterization.ipynb and the #link("https://docs.vulkan.org/spec/latest/chapters/primsrast.html#primsrast-polygons-basic")[vulkan spec].

== Orthographic projection

Now, let $a, b, c in RR$.
Ortho projection is given by
$ mat(a, 0, 0, 0; 0, b, 0, 0; 0, 0, c, 0; 0, 0, 0, 
1) $ describing the function
$ (x, y, z, 1) |-> (a x, b y, c z, 1) $


so in view space, we clip to 

$ -1 <= a x <= 1 $
$ -1 <= b y <= 1 $
$ 0 <= c z <= 1 "(but this one can be disabled with no depth clip)" $ 

== Screen space ambient occlusion

The technique is to use the depth buffer as an approximation: if a point is inside geometry (note: watertight meshes must be used, i.e. ones with a clear inside and outside), then depth of that point is > final 
depth value (nearest geometry), but the converse is not true.

To compute the number of additional pixels needed to ensure proper screen edge rendering, divide the ssao radius by the side length of a single pixel at the near plane, a.k.a. $(n tan (f slash 2)) / ("screen width" slash 2) $ (take ceiling of that).

== Deferred rendering

you should store the final color instead of uvs so that the correct mip level is selected.

casting and conversion: https://learn.microsoft.com/en-us/windows/win32/direct3d9/casting-and-conversion