// Implements all functionality for a 1D Gaussian distribution

use std::ops::{Mul,MulAssign};

#[derive(Debug, Copy, Clone)]
pub struct Gaussian {
    tau: f64,       // precision-mean of the Gaussian
    rho: f64,       // precision of the Gaussian
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

// 
pub fn abs_diff(a: &Gaussian, b: &Gaussian) -> f64 {
    (a.tau - b.tau)
        .abs()
        .max((a.rho - b.rho).abs().sqrt())
}