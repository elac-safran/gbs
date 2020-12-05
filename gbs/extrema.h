#pragma once
#include <nlopt.hpp>
#include <gbs/bscurve.h>
#include <gbs/bssurf.h>
static const char* default_algo = "GN_ORIG_DIRECT";
namespace gbs
{

    template <typename T>
    struct extrema_PC_result
    {
        T u;
        T d;
    };
    template <typename T>
    struct extrema_PS_result
    {
        T u;
        T v;
        T d;
    };
    template <typename T>
    struct extrema_CS_result
    {
        T u_s;
        T v_s;
        T u_c;
        T d;
    };
    /**
     * @brief Project point on curve
     * 
     * @tparam T 
     * @tparam dim 
     * @param crv     : the curve
     * @param pnt     : the point
     * @param u0      : guess value
     * @param tol_x   : tolerance
     * @param solver : solver type please have look to https://nlopt.readthedocs.io/en/latest/NLopt_Algorithms/#nomenclature to change this value
     * @return extrema_PC_result<T> 
     */
    template <typename T, size_t dim>
    auto extrema_PC(const Curve<T, dim> &crv, const std::array<T, dim> &pnt, T u0,T tol_x,const char* solver=default_algo) -> extrema_PC_result<T>
    {

        class UserData
        {
        public:
            UserData(const gbs::Curve<T, dim> &crv, const std::array<T, dim> &pnt) : p{pnt}
            {
                c = &crv;
            }
            const gbs::Curve<T, dim> *c;
            std::array<T, dim> p;
        };
        UserData data(crv, pnt);

        auto f = [](const std::vector<double> &x, std::vector<double> &grad, void *user_data) {
            auto p_d = (UserData *)(user_data);
            auto c_u = p_d->c->value(x[0]);
            if (!grad.empty())
            {
                auto dc_u = p_d->c->value(x[0], 1);
                grad[0] = 0;
                for (int i = 0; i < 3; i++)
                {
                    grad[0] += 2 * dc_u[i] * (c_u[i] - p_d->p[i]);
                }
            }
            return double(gbs::sq_norm(c_u - p_d->p));
        };

        nlopt::opt opt(solver, 1);
        std::vector<double> lb(1), hb(1);
        lb[0] = crv.bounds()[0];
        hb[0] = crv.bounds()[1];

        opt.set_lower_bounds(lb);
        opt.set_upper_bounds(hb);
        opt.set_min_objective(f, &data);
        opt.set_xtol_rel(tol_x);
        std::vector<double> x(1);
        x[0] = u0;
        double minf;

        opt.optimize(x, minf); //can raise

        return {T(x[0]),T(sqrt(minf))};

    }
    /**
     * @brief Project point on curve
     * 
     * @tparam T 
     * @tparam dim 
     * @param crv 
     * @param pnt 
     * @param tol_u 
     * @return auto 
     */
    template <typename T, size_t dim>
    auto extrema_PC(const Curve<T, dim> &crv, const std::array<T, dim> &pnt,T tol_u) -> extrema_PC_result<T>
    {
        auto u0 = T(0.5) * (crv.bounds()[1] - crv.bounds()[0]);
        return extrema_PC(crv, pnt, u0,tol_u);
    }
    /** Project point on surface
     * @brief 
     * 
     * @tparam T 
     * @tparam dim 
     * @tparam rational 
     * @param srf 
     * @param pnt 
     * @param u0 
     * @param v0 
     * @param tol_x 
     * @param solver 
     * @return extrema_PS_result<T> 
     */
    template <typename T, size_t dim>
    auto extrema_PS(const Surface<T, dim> &srf, const std::array<T, dim> &pnt, T u0, T v0, T tol_x, const char *solver = default_algo) -> extrema_PS_result<T>
    {

        class UserData
        {
        public:
            UserData(const Surface<T, dim> &srf, const std::array<T, dim> &pnt) : p{pnt}
            {
                s = &srf;
            }
            const Surface<T, dim> *s;
            std::array<T, dim> p;
        };
        UserData data(srf, pnt);

        auto f = [](const std::vector<T> &x, std::vector<T> &grad, void *user_data) {
            auto p_d = (UserData *)(user_data);
            auto c_uv = p_d->s->value(x[0], x[1]);
            if (!grad.empty())
            {
                // auto dc_u = p_d->c->value(x[0], 1);
                // grad[0] = 0;
                // for (int i = 0; i < 3; i++)
                // {
                //     grad[0] += 2 * dc_u[i] * (c_u[i] - p_d->p[i]);
                // }
                throw std::exception("not implemented");
            }
            return gbs::sq_norm(c_uv - p_d->p);
        };

        nlopt::opt opt(solver, 2);
        std::vector<T> lb(2), hb(2);
        lb[0] = srf.bounds()[0];
        hb[0] = srf.bounds()[1];
        lb[1] = srf.bounds()[2];
        hb[1] = srf.bounds()[3];

        opt.set_lower_bounds(lb);
        opt.set_upper_bounds(hb);
        opt.set_min_objective(f, &data);
        opt.set_xtol_rel(tol_x);
        std::vector<T> x(2);
        x[0] = u0;
        x[1] = v0;
        T minf;

        opt.optimize(x, minf); //can raise

        return {x[0], x[1], sqrt(minf)};
    }
    template <typename T, size_t dim>
    auto extrema_PS(const Surface<T, dim> &srf, const std::array<T, dim> &pnt, T tol_x, const char *solver =default_algo) -> extrema_PS_result<T>
    {
        auto u0  = 0.5 * (srf.bounds()[1] - srf.bounds()[0]);
        auto v0  = 0.5 * (srf.bounds()[3] - srf.bounds()[2]);
        return extrema_PS(srf, pnt, u0, v0, tol_x, solver);
    }

    template <typename T, size_t dim>
    auto extrema_CS(const Surface<T, dim> &srf, const Curve<T, dim> &crv, T u_c0, T u_s0, T v_s0, T tol_x, const char *solver = "LN_COBYLA") -> extrema_CS_result<T>
    {

        class UserData
        {
        public:
            UserData(const Surface<T, dim> &srf, const Curve<T, dim> &crv)
            {
                s = &srf;
                c = &crv;
            }
            const Surface<T, dim> *s;
            const Curve<T, dim> *c;
        };
        UserData data(srf, crv);

        auto f = [](const std::vector<T> &x, std::vector<T> &grad, void *user_data) {
            auto p_d = (UserData *)(user_data);
            auto c_uv = p_d->s->value(x[0], x[1]);
            auto c_u  = p_d->c->value(x[2]);
            if (!grad.empty())
            {
                 throw std::exception("not implemented");
            }
            return gbs::sq_norm(c_uv - c_u);
        };

        nlopt::opt opt(solver, 3);
        std::vector<T> lb(3), hb(3);
        lb[0] = srf.bounds()[0];
        hb[0] = srf.bounds()[1];
        lb[1] = srf.bounds()[2];
        hb[1] = srf.bounds()[3];
        lb[2] = crv.bounds()[0];
        hb[2] = crv.bounds()[1];
        opt.set_lower_bounds(lb);
        opt.set_upper_bounds(hb);
        opt.set_min_objective(f, &data);
        opt.set_xtol_rel(tol_x);
        std::vector<T> x(3);
        x[0] = u_s0;
        x[1] = v_s0;
        x[2] = u_c0;
        T minf;

        opt.optimize(x, minf); //can raise

        return {x[0], x[1], x[2],sqrt(minf)};
    }

    template <typename T, size_t dim>
    auto extrema_CS(const Surface<T, dim> &srf, const Curve<T, dim> &crv, T tol_x, const char *solver = "LN_COBYLA") -> extrema_CS_result<T>
    {
        auto u0  = 0.5 * (srf.bounds()[1] - srf.bounds()[0]);
        auto v0  = 0.5 * (srf.bounds()[3] - srf.bounds()[2]);
        auto u0c = 0.5 * (crv.bounds()[1] - crv.bounds()[0]);
        return extrema_CS(srf, crv, u0, v0, u0c, tol_x, solver);
    }
} // namespace gbs