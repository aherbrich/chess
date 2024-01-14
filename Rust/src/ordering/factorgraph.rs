// Implements all functionality for a factor graph of 1D Gaussian distributions

use super::gaussian::*;
use std::f64::consts::FRAC_1_SQRT_2;
use libm::erfc;
// the types of updates
pub enum Update {
    ToX,
    ToY,
    ToZ,
}

pub trait Factor {
    // each factor needs to implement the update_message function
    fn update_message(&mut self, utype: Update, variables: &mut Vec<Gaussian>) -> f64;
}

// a factor that implements f(x) = N(x; mu, sigma^2)
pub struct GaussianFactor {
    msg_to_x: Gaussian,
    idx_x: usize,
    pub g: Gaussian,
}

impl GaussianFactor {
    // creates a new Gaussian factor
    pub fn new(idx_x: usize, g: Gaussian) -> GaussianFactor {
        GaussianFactor {
            msg_to_x: Gaussian::new(0.0, 0.0),
            idx_x: idx_x,
            g: g,
        }
    }
}

impl Factor for GaussianFactor {
    fn update_message(&mut self, update: Update, variables: &mut Vec<Gaussian>) -> f64 {
        match update {
            Update::ToX => {
                let old_x_marginal = variables[self.idx_x];
                variables[self.idx_x] =  variables[self.idx_x] / self.msg_to_x * self.g;
                self.msg_to_x = self.g;
                abs_diff(&variables[self.idx_x], &old_x_marginal) 
            },
            Update::ToY => panic!("Not supported for the Gaussian factor type"),
            Update::ToZ => panic!("Not supported for the Gaussian factor type"),
        }
        
    }
}

// a factor that implements f(x, y) = N(x; y, beta_squared)
pub struct GaussianMeanFactor {
    msg_to_x: Gaussian,
    msg_to_y: Gaussian,
    idx_x: usize,
    idx_y: usize,
    pub beta_squared: f64,
}

impl GaussianMeanFactor {
    // creates a new Gaussian mean factor
    pub fn new(idx_x: usize, idx_y: usize, beta_squared: f64) -> GaussianMeanFactor {
        GaussianMeanFactor {
            msg_to_x: Gaussian::new(0.0, 0.0),
            msg_to_y: Gaussian::new(0.0, 0.0),
            idx_x: idx_x,
            idx_y: idx_y,
            beta_squared: beta_squared,
        }
    }
}

impl Factor for GaussianMeanFactor {
    fn update_message(&mut self, update: Update, variables: &mut Vec<Gaussian>) -> f64 {
        match update {
            Update::ToX => {
                let msg_back_from_y = variables[self.idx_y] / self.msg_to_y;
                let c = 1.0 / (1.0 + self.beta_squared * msg_back_from_y.rho);
                let new_msg_to_x = Gaussian::new(c * msg_back_from_y.tau, c * msg_back_from_y.rho);
            
                let old_x_marginal = variables[self.idx_x];
                variables[self.idx_x] = old_x_marginal / self.msg_to_x * new_msg_to_x;
                self.msg_to_x = new_msg_to_x;
                abs_diff(&variables[self.idx_x], &old_x_marginal)
            },
            Update::ToY => {
                let msg_back_from_x = variables[self.idx_x] / self.msg_to_x;
                let c = 1.0 / (1.0 + self.beta_squared * msg_back_from_x.rho);
                let new_msg_to_y = Gaussian::new(c * msg_back_from_x.tau, c * msg_back_from_x.rho);
            
                let old_y_marginal = variables[self.idx_y];
                variables[self.idx_y] = old_y_marginal / self.msg_to_y * new_msg_to_y;
                self.msg_to_y = new_msg_to_y;
                abs_diff(&variables[self.idx_y], &old_y_marginal)
            },                
            Update::ToZ => panic!("Not supported for the Gaussian mean factor type"),
        }
        
    }
}

// a factor that implements f(x, y, z) = \delta(z - a*x + b*y)
pub struct WeightedSumFactor {
    msg_to_x: Gaussian,
    msg_to_y: Gaussian,
    msg_to_z: Gaussian,
    idx_x: usize,
    idx_y: usize,
    idx_z: usize,
    pub a1: f64,
    pub a2: f64,
}

impl WeightedSumFactor {
    // creates a new weighted sum factor
    pub fn new(idx_x: usize, idx_y: usize,  idx_z: usize, a1: f64, a2: f64) -> WeightedSumFactor {
        WeightedSumFactor {
            msg_to_x: Gaussian::new(0.0, 0.0),
            msg_to_y: Gaussian::new(0.0, 0.0),
            msg_to_z: Gaussian::new(0.0, 0.0),
            idx_x: idx_x,
            idx_y: idx_y,
            idx_z: idx_z,
            a1: a1,
            a2: a2,
        }
    }
}

impl Factor for WeightedSumFactor {
    fn update_message(&mut self, update: Update, variables: &mut Vec<Gaussian>) -> f64 {
        match update {
            Update::ToX => {
                let msg_back_from_y = variables[self.idx_y] / self.msg_to_y;
                let msg_back_from_z = variables[self.idx_z] / self.msg_to_z;
                let new_msg_to_x = Gaussian::new_from_mean_and_variance(
                    (msg_back_from_z.mean() - self.a2 * msg_back_from_y.mean()) / self.a1,
                    (msg_back_from_z.variance() + self.a2 * self.a2 * msg_back_from_y.variance()) / (self.a1 * self.a1),
                );
            
                let old_x_marginal = variables[self.idx_x];
                variables[self.idx_x] = old_x_marginal / self.msg_to_x * new_msg_to_x;
                self.msg_to_x = new_msg_to_x;
                abs_diff(&variables[self.idx_x], &old_x_marginal)
            },
            Update::ToY => {
                let msg_back_from_x = variables[self.idx_x] / self.msg_to_x;
                let msg_back_from_z = variables[self.idx_z] / self.msg_to_z;
                let new_msg_to_y = Gaussian::new_from_mean_and_variance(
                    (msg_back_from_z.mean() - self.a1 * msg_back_from_x.mean()) / self.a2,
                    (msg_back_from_z.variance() + self.a1 * self.a1 * msg_back_from_x.variance()) / (self.a2 * self.a2),
                );
            
                let old_y_marginal = variables[self.idx_y];
                variables[self.idx_y] = old_y_marginal / self.msg_to_y * new_msg_to_y;
                self.msg_to_y = new_msg_to_y;
                abs_diff(&variables[self.idx_y], &old_y_marginal)
            },                
            Update::ToZ => {
                let msg_back_from_x = variables[self.idx_x] / self.msg_to_x;
                let msg_back_from_y = variables[self.idx_y] / self.msg_to_y;
                let new_msg_to_z = Gaussian::new_from_mean_and_variance(
                    self.a1 * msg_back_from_x.mean() + self.a2 * msg_back_from_y.mean(),
                    self.a1 * self.a1 * msg_back_from_x.variance() + self.a2 * self.a2 * msg_back_from_y.variance(),
                );
            
                let old_z_marginal = variables[self.idx_z];
                variables[self.idx_z] = old_z_marginal / self.msg_to_z * new_msg_to_z;
                self.msg_to_z = new_msg_to_z;
                abs_diff(&variables[self.idx_z], &old_z_marginal)
            },
        }
        
    }
}

// a factor that implements f(x) = I(x >= 0)
pub struct GreaterThanFactor {
    msg_to_x: Gaussian,
    idx_x: usize,
    pub epsilon: f64,
}

impl GreaterThanFactor {
    // creates a new greater-than factor
    pub fn new(idx_x: usize, epsilon: f64) -> GreaterThanFactor {
        GreaterThanFactor {
            msg_to_x: Gaussian::new(0.0, 0.0),
            idx_x: idx_x,
            epsilon: epsilon,
        }
    }
}

impl Factor for GreaterThanFactor {
    fn update_message(&mut self, update: Update, variables: &mut Vec<Gaussian>) -> f64 {
        match update {
            Update::ToX => {
                let msg_back_from_x = variables[self.idx_x] / self.msg_to_x;
                let sqrt_rho = msg_back_from_x.rho.sqrt();
                let a = msg_back_from_x.tau / sqrt_rho;
                let b = self.epsilon * sqrt_rho;
                let x = a - b;
                let v = 0.3989422804014327 * (-x * x / 2.0).exp() / (0.5 * erfc(-x * FRAC_1_SQRT_2));
                let w = v * (v + x);
                let c = 1.0 - w;
                let old_x_marginal = variables[self.idx_x];
                variables[self.idx_x] = Gaussian::new(
                    (msg_back_from_x.tau + sqrt_rho * v) / c,
                     msg_back_from_x.rho / c,
                );
                self.msg_to_x = variables[self.idx_x] / msg_back_from_x;
                abs_diff(&variables[self.idx_x], &old_x_marginal) 
            },
            Update::ToY => panic!("Not supported for the Gaussian factor type"),
            Update::ToZ => panic!("Not supported for the Gaussian factor type"),
        }
        
    }
}


// an implementation of a factor graph
pub struct FactorGraph {
    pub variables: Vec<Gaussian>,
    factors: Vec<Box<dyn Factor>>,
}

impl FactorGraph {
    // creates a new factor graph
    pub fn new() -> FactorGraph {
        FactorGraph {
            variables: Vec::new(),
            factors: Vec::new(),
        }
    }

    // returns the marginal of a variable
    pub fn get_marginal(&self, variable: usize) -> Gaussian {
        self.variables[variable]
    }

    // adds a new variable to the factor graph
    pub fn new_variable(&mut self) -> usize {
        let index = self.variables.len();
        self.variables.push(Gaussian::new(0.0, 0.0));
        index
    }

    // adds a new factor to the factor graph
    pub fn new_factor(&mut self, factor: Box<dyn Factor>) -> usize {
        let index = self.factors.len();
        self.factors.push(factor);
        index
    }
}