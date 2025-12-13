
template <int N>
struct vec
{
   float data[N];

   float& operator[](int i)
   {
      return data[i];
   }

   void operator+=(vec other)
   {
      *this = *this + other;
   }
   void operator-=(vec other)
   {
      *this = *this - other;
   }
};

template <int N>
vec<N> operator-(vec<N> v)
{
   vec<N> res;
   range (i, N) res[i] = -v[i];
   return res;
}

template <int N>
vec<N> operator+(vec<N> v, vec<N> w)
{
   vec<N> res;
   range (i, N) res[i] = v[i] + w[i];
   return res;
}

template <int N>
vec<N> operator-(vec<N> v, vec<N> w)
{
   vec<N> res;
   range (i, N) res[i] = v[i] - w[i];
   return res;
}

template <int N>
vec<N> operator*(vec<N> v, float factor)
{
   vec<N> res;
   range (i, N) res[i] = v[i] * factor;
   return res;
}

template <int N>
vec<N> operator*(float factor, vec<N> v)
{
   vec<N> res;
   range (i, N) res[i] = factor * v[i];
   return res;
}

template <int N>
vec<N> operator/(vec<N> v, float factor)
{
   vec<N> res;
   range (i, N) res[i] = v[i] / factor;
   return res;
}

template <int N>
float dot(vec<N> v, vec<N> w)
{
   float res = 0;
   range (i, N) res += v[i] * w[i];
   return res;
}

template <int N>
float length(vec<N> v)
{
   return sqrt(dot(v, v));
}

template <int N>
vec<N> normalize(vec<N> v)
{
   return v / length(v);
}

template <int N>
vec<N> lerp(vec<N> v, vec<N> w, float t)
{
   return v + (w - v) * t;
}

using vec2 = vec<2>;
using vec3 = vec<3>;
using vec4 = vec<4>;

bool eq(float x, float y)
{
   return abs(x - y) <= max(0.0001f, 0.0001f * max(abs(x), abs(y)));
}

template <int N>
bool eq(vec<N> v, vec<N> w)
{
   range (i, N)
      if (!eq(v[i], w[i]))
         return false;
   return true;
}

vec3 cross(vec3 v, vec3 w)
{
   return {
       v[1] * w[2] - v[2] * w[1],
       v[2] * w[0] - v[0] * w[2],
       v[0] * w[1] - v[1] * w[0]};
}

template <int N, int M>
struct mat
{
   vec<M> data[N];

   vec<M>& operator[](int i)
   {
      return data[i];
   }
};

using mat4 = mat<4, 4>;
using mat3x4 = mat<3, 4>;
using mat3 = mat<3, 3>;
using mat3x2 = mat<3, 2>;
using mat2x3 = mat<2, 3>;
using mat2 = mat<2, 2>;

template <int N, int M>
mat<N, M> operator-(mat<N, M> A)
{
   mat<N, M> res;
   range (i, N)
      range (j, M)
         res[i][j] = -A[i][j];

   return res;
}

template <int N, int M>
vec<N> operator*(mat<N, M> A, vec<M> v)
{
   vec<N> res = {};
   range (i, N)
      range (j, M)
         res[i] += A[i][j] * v[j];
   return res;
}
template <int N, int M, int P>
mat<N, P> operator*(mat<N, M> A, mat<M, P> B)
{
   mat<N, P> res = {};
   range (i, N)
      range (j, P)
         range (k, M)
            res[i][j] += A[i][k] * B[k][j];
   return res;
}

template <int N, int M>
mat<N, M> operator/(mat<N, M> A, float factor)
{
   mat<N, M> res;
   range (i, N)
      range (j, M)
         res[i][j] = A[i][j] / factor;
   return res;
}

template <int N, int M>
mat<M, N> transpose(mat<N, M> A)
{
   mat<M, N> res;
   range (i, N)
      range (j, M)
         res[j][i] = A[i][j];
   return res;
}

mat2 inverse(mat2 A)
{
   float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
   return mat2{
              A[1][1], -A[0][1],
              -A[1][0], A[0][0]} /
          det;
}

// https://math.stackexchange.com/questions/632525/if-a-is-full-column-rank-then-ata-is-always-invertible
// https://www.physicsforums.com/threads/left-and-right-inverses-of-a-non-square-matrix.1001138/
mat2x3 leftInverse(mat3x2 A)
{
   return inverse(transpose(A) * A) * transpose(A);
}

mat3x2 columns(vec3 v, vec3 w)
{
   return {
       v[0], w[0],
       v[1], w[1],
       v[2], w[2]};
}

mat3 euler(float y, float p, float r)
{
   return {sin(p) * sin(r) * sin(y) + cos(r) * cos(y), sin(p) * sin(y) * cos(r) - sin(r) * cos(y), sin(y) * cos(p),
           sin(r) * cos(p), cos(p) * cos(r), -sin(p),
           sin(p) * sin(r) * cos(y) - sin(y) * cos(r), sin(p) * cos(r) * cos(y) + sin(r) * sin(y), cos(p) * cos(y)};
}

mat3 scale(float x, float y, float z)
{
   return {x, 0, 0,
           0, y, 0,
           0, 0, z};
}

mat4 affine(mat3 A, vec3 v)
{
   return {
       A[0][0], A[0][1], A[0][2], v[0],
       A[1][0], A[1][1], A[1][2], v[1],
       A[2][0], A[2][1], A[2][2], v[2],
       0, 0, 0, 1};
}

template <int N>
mat<N, N> id()
{
   mat<N, N> res = {};
   range (i, N)
      res[i][i] = 1;
   return res;
}
float square(float x)
{
   return x * x;
}
