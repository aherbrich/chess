// Implements all functionality for a 1D Gaussian distribution

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

    // computes the absolute difference between two 1D Gaussian
    pub fn abs_diff(&self, other: &Gaussian) -> f64 {
        (self.tau - other.tau)
            .abs()
            .max((self.rho - other.rho).abs().sqrt())
    }

}