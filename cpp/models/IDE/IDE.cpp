#include "IDE/IDE.h"
#include <iostream>

namespace mio{
    /**
    * Constructor of IDEModel
    * @param init Vector with 3d Vectors in its entries containing the initial (time, quantity of susceptible, R0t) values
    *   the time values should have a distance of dt_init; we need time values from -(k-1)*dt until 0 -> condition: length_init>= k
    *   (with k=std::ceil((timeinfectious+timelatency)/dt)) with default values: take 11.6 and you are safe
    * @param length_init length of the vector containing the inital values
    * @param dt_init size of the time step used for numerical integration
    * @param N population of the considered Region 
    */
    IdeModel::IdeModel(std::vector<Eigen::Vector2d> init, int length_init, double dt_init, int N_init)
    :result(init),length_result(length_init), dt(dt_init),N(N_init){
        l=(int) std::floor(timelatency/dt);
        k=(int) std::ceil((timeinfectious+timelatency)/dt);
    }

    /**
    * Change latencytime, default value: 3.3
    * @param latency The value of the new latency time
    */
    void IdeModel::set_latencytime(double latency){
        timelatency=latency;
        //initialize parameters k and  l used for numerical calculations in the simulation
        l=(int) std::floor(timelatency/dt);
        k=(int) std::ceil((timeinfectious+timelatency)/dt);
    }

    /**
    * Change infectioustime, default value: 8.2
    * @param infectious The value of the new infectious time
    */
    void IdeModel::set_infectioustime(double infectious){
        timeinfectious=infectious;
        k=(int) std::ceil((timeinfectious+timelatency)/dt);
    }

    /**
    * Density of the generalized beta distribution used for the function f_{beta} of the IDGL model.
    * 
    * @param tau: Function  of 
    * @param p parameter p of the generalized Beta distribution
    * @param q parameter q of the generalized Beta distribution
    */
    double IdeModel::Beta(double tau, double p, double q) const{
        if((timelatency<tau) && (timeinfectious+timelatency>tau)){
            return tgamma(p+q)*pow(tau-timelatency,p-1)*pow(timeinfectious+timelatency-tau,q-1)/
                (tgamma(p)*tgamma(q)*pow(timeinfectious,p+q-1));
        }
        return 0.0;
    }

    /**
    * Numerical differentiation of S using a central difference quotient.
    * @param idx Time index at which the numerical differentiation is to be performed
    * @return S'(t[idx]) numerically approximated
    */
    double IdeModel::S_derivative(int idx) const{
        return (result[idx+1][1]-result[idx-1][1])/(2*dt);
    }

    /**
    * Numerical integration of the inner integral of the integro-differential equation for the group S using a trapezoidal sum.
    * @param idx Index of the time in the inner integral 
    * @return Result of the numerical integration
    */
    double IdeModel::num_integration_inner_integral(int idx) const{
        double res=0.5 * (Beta(result[idx][0] -result[idx-k][0]) * S_derivative(k) + 
                Beta(result[idx][0]-result[idx-l][0]) * S_derivative(idx-l));
        int i=idx-k+1;
        while (i<=idx-l-2){
            res+=(Beta(result[idx][0]-result[i][0])*S_derivative(i));
            i++;
        }
        return res;
    }

    /**
    * Simulation of the course of infection
    * @param t_max last simulation day
    * @return result of the simulation, stored in an vector with 2d vectors in each entry. 
    *   These 2d vectors contains the time values in the first entry and the S (susceptible) values in the second one.
    */
    std::vector<Eigen::Vector2d> IdeModel::simulate(int t_max){
        double R0t_last=1.0;
        double R0t_current=1.0;
        int idx_R0t=0;
        bool next=false;
        //set initial R0t
        if (idx_R0t<length_R0t && R0t[idx_R0t][0]<=0.0){
            
            std::cout<<"here"<<std::endl;
            R0t_last=R0t[idx_R0t][1];
            R0t_current=R0t[idx_R0t][1];
            idx_R0t++;
        }

        while (result[length_result-1][0]<t_max){
            //change last R0t if it was changed in the last timestep
            if(next){
                next=false;
                R0t_last=R0t_current;
            }

            if(idx_R0t<length_R0t && R0t[idx_R0t][0]<=round((result[length_result-1][0]+dt)*10000.0)/10000.0){
                R0t_current=R0t[idx_R0t][1];
                idx_R0t++;
                // remembering, that R0t_last has to be changed in the next time step
                next=true;
                //std::cout<<"here"<<std::endl;
            }

            result.push_back(Eigen::Vector2d (round((result[length_result-1][0]+dt)*10000.0)/10000.0,0));
            length_result++;
            result[length_result-1][1]=result[length_result-2][1] * exp(
                    dt/(2*N)*(R0t_last* num_integration_inner_integral(length_result-2)+
                    R0t_current* num_integration_inner_integral(length_result-1)));
        }
        return result;
    }

    /**
    * add a damping to simulate a new NPI;
    * dampings has to be added in ascending chronological order 
    * @param time time at which new damping should start. prerequisite: time>=0
    * @param R0t_time new R0t value. Is used from "time" until next time, a damping is set
    */
    void IdeModel::add_damping(double time, double R0t_time){
        R0t.push_back(Eigen::Vector2d(time,R0t_time)); 
        length_R0t++;
    }

    /**
    * print simulation result
    */
    void IdeModel::print_result() const{
        std::cout<<"time  |  number of susceptibles"<<std::endl;
        for (int i=0;i<length_result;i++){
            std::cout<<result[i][0]<<"  |  "<<result[i][1]<<std::endl;
        }
        
    }
}