#ifndef DAMPING_H
#define DAMPING_H

#include "epidemiology/utils/eigen.h"
#include "epidemiology/utils/type_safe.h"
#include "epidemiology/utils/stl_util.h"
#include "epidemiology/utils/matrix_shape.h"
#include "epidemiology/math/smoother.h"

#include <tuple>
#include <vector>
#include <algorithm>
#include <ostream>

namespace epi
{

/**
 * integer damping level.
 */
DECL_TYPESAFE(int, DampingLevel);

/**
 * integer damping type.
 */
DECL_TYPESAFE(int, DampingType);

/**
 * double simulation time.
 */
class SimulationTime : public TypeSafe<double, SimulationTime>,
                       public OperatorAdditionSubtraction<SimulationTime>,
                       public OperatorScalarMultiplicationDivision<SimulationTime, double>,
                       public OperatorComparison<SimulationTime>
{
public:
    using TypeSafe<double, SimulationTime>::TypeSafe;
};

/**
 * represent interventions or effects that affect contact frequencies between multiple groups.
 * Dampings have a level and a type and are active from a certain point in time forward.
 * Dampings are square matrix valued, coefficient d_ij affects the contacts from group i to group j.
 * @tparam S Matrix shape type
 */
template<class S>
class Damping : public std::tuple<typename S::Matrix, DampingLevel, DampingType, SimulationTime>
{
public:
    using Shape = S;
    using Matrix = typename Shape::Matrix;
    using Base = std::tuple<Matrix, DampingLevel, DampingType, SimulationTime>;

    /**
     * create a default Damping.
     * @param shape_args arguments to construct the shape of the damping matrix (can be Shape itself, copy ctor)
     * @tparam T constructor arguments of Damping::Shape.
     */
    template<class... T, class = std::enable_if_t<std::is_constructible<Shape, T...>::value, int>>
    explicit Damping(T... shape_args)
        : Base(Matrix::Zero(Shape(shape_args...).rows(), Shape(shape_args...).cols()), {}, {}, {})
    {
    }

    /**
     * create a Damping.
     * @param m matrix of damping coefficients
     * @param level damping level
     * @param type damping type
     * @param t time at which the damping becomes active
     * @tparam ME matrix expression, must be compatible with Shape
     */
    template <class ME, class = std::enable_if_t<is_matrix_expression<std::decay_t<ME>>::value, void>>
    Damping(ME&& m, DampingLevel level, DampingType type, SimulationTime t)
        : Base(std::forward<ME>(m), level, type, t)
    {
        assert((get_coeffs().array() >= 0.).all() && (get_coeffs().array() <= 1.).all() && "damping coefficient out of range");
    }

    /**
     * create a Damping with constant coefficients.
     * @param shape_args arguments to construct the shape of the damping matrix (can be Shape itself, copy ctor)
     * @param d damping coefficient for all groups.
     * @param level damping level
     * @param type damping type
     * @param t time at which the damping becomes active
     * @tparam T Shape constructor arguments.
     */
    template<class... T, class = std::enable_if_t<std::is_constructible<Shape, T...>::value, void>>
    Damping(double d, DampingLevel level, DampingType type, SimulationTime t, T... shape_args)
        : Damping(Matrix::Constant(Shape(shape_args...).rows(), Shape(shape_args...).cols(), d), level, type, t)
    {
    }

    /**
     * create a Damping at level and type zero
     * @param m damping coefficients
     * @param t time at which the damping becomes active
     * @tparam ME matrix expression, must be compatible with Damping::Matrix
     */
    template <class ME, class = std::enable_if_t<is_matrix_expression<std::decay_t<ME>>::value, void>>
    Damping(ME&& m, SimulationTime t)
        : Damping(std::forward<ME>(m), DampingLevel(0), DampingType(0), t)
    {
    }

    /**
     * create a Damping with constant coefficients and zero level and type.
     * @param shape_args arguments to construct the shape of the damping matrix (can be Shape itself, copy ctor)
     * @param d damping coefficient for all groups.
     * @param t time at which the damping becomes active
     * @tparam T Shape constructor arguments.
     */
    template<class... T, class = std::enable_if_t<std::is_constructible<Shape, T...>::value, void>>
    Damping(double d, SimulationTime t, T... shape_args)
        : Damping(d, DampingLevel(0), DampingType(0), t, shape_args...)
    {
    }

    /**
     * the time this damping becomes active.
     */
    SimulationTime& get_time()
    {
        return std::get<SimulationTime>(*this);
    }
    const SimulationTime& get_time() const
    {
        return std::get<SimulationTime>(*this);
    }

    /**
     * the level of this damping.
     */
    DampingLevel& get_level()
    {
        return std::get<DampingLevel>(*this);
    }
    const DampingLevel& get_level() const
    {
        return std::get<DampingLevel>(*this);
    }

    /**
     * the type of this damping.
     */
    DampingType& get_type()
    {
        return std::get<DampingType>(*this);
    }
    const DampingType& get_type() const
    {
        return std::get<DampingType>(*this);
    }

    /**
     * the coefficients of this damping.
     */
    const Matrix& get_coeffs() const
    {
        return std::get<Matrix>(*this);
    }
    Matrix& get_coeffs()
    {
        return std::get<Matrix>(*this);
    }

    /**
     * shape of the damping matrix.
     */
    Shape get_shape() const
    {
        return Shape::get_shape_of(get_coeffs());
    }
};

/**
 * collection of dampings at different time points.
 * combination of dampings is computed in the way described at get_matrix_at.
 * @see get_matrix_at 
 * @tparam D an instance of Damping template or compatible type. 
 */
template<class D>
class Dampings
{
public:
    using Shape           = typename D::Shape;
    using Matrix          = typename Shape::Matrix;
    using value_type      = D;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using iterator        = typename std::vector<value_type>::iterator;
    using const_iterator  = typename std::vector<value_type>::const_iterator;

    /**
     * create damping collection.
     * @param shape_args shape constructor arguments.
     * @param num_dampings number of initial elements in the collection
     * @tparam T Shape constructor arguments.
     */
    template<class... T, class = std::enable_if_t<std::is_constructible<Shape, T...>::value, void>>
    explicit Dampings(T... shape_args)
        : m_dampings()
        , m_shape(shape_args...)
    {
    }

    /**
     * create damping collection.
     * @param il initializer list of Dampings
     */
    Dampings(std::initializer_list<value_type> il)
        : m_dampings(il)
    {
        assert(il.size() > 0);
        m_shape = il.begin()->get_shape();
    }

    /**
     * add a damping.
     * @param damping a Damping
     */
    void add(const value_type& damping)
    {
        add_(damping);
    }
    template<class... T>
    void add(T&&... t)
    {
        add_(value_type(std::forward<T>(t)...));
    }
    template<class...T>
    void add(double d, T... t)
    {
        add_(value_type(d, std::forward<T>(t)..., m_shape));
    }

    /**
     * Computes the real contact frequency at a point in time.
     * Combines the dampings that are active at a point in time according to
     * type and level rules. Dampings on different levels apply "multiplicatively".
     * Dampings on the same level apply additively.
     * e.g.
     * Two dampings a and b on different levels combine to (1 - (1 - a)(1 - b)) or (a + b - ab).
     * Two dampings a and b on the same level combine to (a + b). 
     * 
     * Transitions between different contact frequencies are smoothed out over one day to avoid discontinuities.
     * Uses lazy evaluation, coefficients are calculated on indexed access.
     * @param t time in the simulation
     * @return matrix expression 
     */
    auto get_matrix_at(SimulationTime t) const
    {
        finalize();
        auto ub =
            std::upper_bound(m_accumulated_dampings_cached.begin(), m_accumulated_dampings_cached.end(),
                             std::make_tuple(t), [](auto&& tup1, auto&& tup2) {
                                 return double(std::get<SimulationTime>(tup1)) < double(std::get<SimulationTime>(tup2));
                             });
        auto damping =
            smoother_cosine(double(t), double(std::get<SimulationTime>(*ub)) - 1, double(std::get<SimulationTime>(*ub)),
                            std::get<Matrix>(*(ub - 1)), std::get<Matrix>(*ub));
        return damping;
    }
    auto get_matrix_at(double t) const
    {
        return get_matrix_at(SimulationTime(t));
    }

    /**
     * compute the cache of accumulated dampings.
     * if this is used after adding dampings, all subsequent calls to get_matrix_at()
     * are quick and threadsafe. Otherwise the cache is updated automatically on the first call.
     */
    void finalize() const;

    /**
     * access one damping in this collection.
     */
    reference operator[](size_t i)
    {
        return m_dampings[i];
    }
    const_reference operator[](size_t i) const
    {
        return m_dampings[i];
    }

    /**
     * equality operators.
     */
    bool operator==(const Dampings& other) const
    {
        return m_dampings == other.m_dampings;
    }
    bool operator!=(const Dampings& other) const
    {
        return !(*this == other);
    }

    /**
     * get the number of matrices.
     */
    size_t get_num_dampings() const
    {
        return m_dampings.size();
    }

    /**
     * dimensions of the damping matrix.
     */
    Shape get_shape() const
    {
        return m_shape;
    }

    /**
     * STL iterators over matrices.
     */
    iterator begin()
    {
        return m_dampings.begin();
    }
    iterator end()
    {
        return m_dampings.end();
    }
    const_iterator begin() const
    {
        return m_dampings.begin();
    }
    const_iterator end() const
    {
        return m_dampings.end();
    }

    /**
     * GTest printer.
     */
    friend void PrintTo(const Dampings& self, std::ostream* os)
    {
        for (auto& d : self.m_dampings) {
            *os << '\n'
                << '[' << std::get<SimulationTime>(d) << ',' << std::get<DampingType>(d) << ','
                << std::get<DampingLevel>(d) << ']';
            *os << '\n' << std::get<Matrix>(d);
        }
    }

private:
    /**
     * internal add.
     */
    void add_(const value_type& damping);

    /**
     * replace matrices of the same type, sum up matrices on the same level.
     * add new types/levels if necessary.
     */
    static void update_active_dampings(
        const value_type& damping,
        std::vector<std::tuple<std::reference_wrapper<const Matrix>, DampingLevel, DampingType>>&
            active_by_type,
        std::vector<std::tuple<Matrix, DampingLevel>>& sum_by_level);

    /**
     * e.g. inclusive_exclusive_sum({A, B, C}) = A + B + C - AB - BC - AC + ABC
     * equal to but more efficient than 1 - (1 - A)(1 - B)(1 - C))
     */
    template <class Iter>
    static void inclusive_exclusive_sum_rec(Iter b, Iter e, Matrix& sum)
    {
        if (b != e) {
            sum = sum + std::get<Matrix>(*b) - (sum.array() * std::get<Matrix>(*b).array()).matrix();
            inclusive_exclusive_sum_rec(++b, e, sum);
        }
    }
    template <class Tuple>
    static Matrix inclusive_exclusive_sum(const std::vector<Tuple>& v)
    {
        assert(!v.empty());
        auto& m  = std::get<Matrix>(v.front());
        auto sum = m.eval();
        inclusive_exclusive_sum_rec(v.begin() + 1, v.end(), sum);
        return sum;
    }

private:
    std::vector<value_type> m_dampings;
    Shape m_shape;
    mutable std::vector<std::tuple<Matrix, SimulationTime>> m_accumulated_dampings_cached;
};

template<class S>
void Dampings<S>::finalize() const
{
    using std::get;

    if (m_accumulated_dampings_cached.empty()) {
        m_accumulated_dampings_cached.emplace_back(Matrix::Zero(m_shape.rows(), m_shape.cols()),
                                                   SimulationTime(std::numeric_limits<double>::lowest()));

        std::vector<std::tuple<std::reference_wrapper<const Matrix>, DampingLevel, DampingType>>
            active_by_type;
        std::vector<std::tuple<Matrix, DampingLevel>> sum_by_level;
        for (auto& damping : m_dampings) {
            update_active_dampings(damping, active_by_type, sum_by_level);
            m_accumulated_dampings_cached.emplace_back(inclusive_exclusive_sum(sum_by_level),
                                                       get<SimulationTime>(damping));
            assert((get<Matrix>(m_accumulated_dampings_cached.back()).array() <= 1).all() &&
                   (get<Matrix>(m_accumulated_dampings_cached.back()).array() >= 0).all() &&
                   "unexpected error, accumulated damping out of range.");
        }

        m_accumulated_dampings_cached.emplace_back(get<Matrix>(m_accumulated_dampings_cached.back()),
                                                   SimulationTime(std::numeric_limits<double>::max()));
    }
}

template<class D>
void Dampings<D>::add_(const value_type& damping)
{
    assert(damping.get_shape() == m_shape);
    insert_sorted_replace(m_dampings, damping, [](auto& tup1, auto& tup2) {
        return double(std::get<SimulationTime>(tup1)) < double(std::get<SimulationTime>(tup2));
    });
    m_accumulated_dampings_cached.clear();
}

template<class S>
void Dampings<S>::update_active_dampings(
    const value_type& damping,
    std::vector<std::tuple<std::reference_wrapper<const Matrix>, DampingLevel, DampingType>>& active_by_type,
    std::vector<std::tuple<Matrix, DampingLevel>>& sum_by_level)
{
    using std::get;

    const int MatrixIdx = 0;

    auto iter_active_same_type = std::find_if(active_by_type.begin(), active_by_type.end(), [&damping](auto& active) {
        return get<DampingLevel>(active) == get<DampingLevel>(damping) &&
               get<DampingType>(active) == get<DampingType>(damping);
    });
    if (iter_active_same_type != active_by_type.end()) {
        //replace active of the same type and level
        auto& active_same_type = *iter_active_same_type;
        auto& sum_same_level   = *std::find_if(sum_by_level.begin(), sum_by_level.end(), [&damping](auto& sum) {
            return get<DampingLevel>(sum) == get<DampingLevel>(damping);
        });
        get<MatrixIdx>(sum_same_level) += get<MatrixIdx>(damping) - get<MatrixIdx>(active_same_type).get();
        get<MatrixIdx>(active_same_type) = get<MatrixIdx>(damping);
    }
    else {
        //add new type
        active_by_type.emplace_back(get<MatrixIdx>(damping), get<DampingLevel>(damping), get<DampingType>(damping));

        auto iter_sum_same_level = std::find_if(sum_by_level.begin(), sum_by_level.end(), [&damping](auto& sum) {
            return get<DampingLevel>(sum) == get<DampingLevel>(damping);
        });
        if (iter_sum_same_level != sum_by_level.end()) {
            //add to existing level
            get<MatrixIdx>(*iter_sum_same_level) += get<MatrixIdx>(damping);
        }
        else {
            //add new level
            sum_by_level.emplace_back(get<MatrixIdx>(damping), get<DampingLevel>(damping));
        }
    }
}

/**
 * aliases for common damping specializations.
 */
using SquareDamping = Damping<SquareMatrixShape>;
using SquareDampings = Dampings<SquareDamping>;
using VectorDamping = Damping<ColumnVectorShape>;
using VectorDampings = Dampings<VectorDamping>;

} // namespace epi

#endif // DAMPING_H
