// Implements all functionality for a 1D Gaussian distribution

use std::ops::{Mul,MulAssign,Div,DivAssign};
use std::fmt;
use std::f64::consts::PI;
use std::f64;

#[derive(Debug,Copy,Clone)]
pub struct Gaussian {
    pub tau: f64,       // precision-mean of the Gaussian
    pub rho: f64,       // precision of the Gaussian
}

impl Gaussian {
    // creates a new 1D Gaussian
    pub fn new(tau: f64, rho: f64) -> Gaussian {
        Gaussian {
            tau: tau,
            rho: rho,
        }
    }

    // creates a new 1D Gaussian from a mean and variance
    pub fn new_from_mean_and_variance(mean: f64, variance: f64) -> Gaussian {
        Gaussian {
            tau: mean / variance,
            rho: 1.0 / variance,
        }
    }

    // creates a new 1D Gaussian with zero mean and variance of 1
    pub fn new_standard_normal() -> Gaussian {
        Gaussian {
            tau: 0.0,
            rho: 1.0,
        }
    }

    // computes the mean of a 1D Gaussian
    pub fn mean(&self) -> f64 {
        self.tau / self.rho
    }

    // computes the variance of a 1D Gaussian
    pub fn variance(&self) -> f64 {
        1.0 / self.rho
    }
}

impl Mul for Gaussian {
    type Output = Self;

    // multiplies two 1D Gaussian
    fn mul(self, other: Gaussian) -> Gaussian {
        Gaussian {
            tau: self.tau + other.tau,
            rho: self.rho + other.rho,
        }
    }
}

impl MulAssign for Gaussian {
    // multiplies two 1D Gaussian in-place
    fn mul_assign(&mut self, other: Gaussian) {
        self.tau += other.tau;
        self.rho += other.rho;
    }
}

impl Div for Gaussian {
    type Output = Self;

    // divides two 1D Gaussian
    fn div(self, other: Gaussian) -> Gaussian {
        Gaussian {
            tau: self.tau - other.tau,
            rho: self.rho - other.rho,
        }
    }
}   

impl DivAssign for Gaussian {
    // divides two 1D Gaussian in-place
    fn div_assign(&mut self, other: Gaussian) {
        self.tau -= other.tau;
        self.rho -= other.rho;
    }
}

impl fmt::Display for Gaussian {
    // formats a 1D Gaussian
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        if self.rho == 0.0 {
            write!(f, "(μ = 0, σ = Inf)")
        } else {
            write!(f, "(μ = {0:.3}, σ = {1:.3})", self.mean(), self.variance().sqrt())
        }        
    }
}

// computes the absolute difference between two 1D Gaussian
pub fn abs_diff(a: &Gaussian, b: &Gaussian) -> f64 {
    (a.tau - b.tau)
        .abs()
        .max((a.rho - b.rho).abs().sqrt())
}

// returns the log-normalization constant of the product of two 1D Gaussian 
pub fn log_norm_product(g1: &Gaussian, g2: &Gaussian) -> f64 {
    if g1.rho == 0.0 || g2.rho == 0.0 {
        0.0
    } else {
        let var_sum = g1.variance() + g2.variance();
        let mean_diff = g1.mean() - g2.mean();
        -0.5 * ((2.0 * PI * var_sum).ln() + mean_diff * mean_diff / var_sum)
    }
}

// returns the log-normalization constant of the ratio of two 1D Gaussian 
pub fn log_norm_ratio(g1: &Gaussian, g2: &Gaussian) -> f64 {
    if g1.rho == 0.0 || g2.rho == 0.0 {
        0.0
    } else {
        let var_g2 = g2.variance();
        let var_diff = var_g2 - g1.variance();
        if var_diff == 0.0 {
            0.0
        } else {
            let mean_diff = g1.mean() - g2.mean();
            var_g2.ln() + 0.5 * ((2.0 * PI / var_diff).ln() + mean_diff * mean_diff / var_diff)
        }
    }
}
