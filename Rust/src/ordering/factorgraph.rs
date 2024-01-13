// Implements all functionality for a factor graph of 1D Gaussian distributions

use super::gaussian::*;

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

// a factor that implements f(x) = g(x)
pub struct GaussianFactor {
    msg_to_x: Gaussian,
    idx_x: usize,
    pub g: Gaussian,
}

impl GaussianFactor {
    // creates a new Gaussian factor
    pub fn new(variable: usize, g: Gaussian) -> GaussianFactor {
        GaussianFactor {
            msg_to_x: Gaussian::new(0.0, 0.0),
            idx_x: variable,
            g: g,
        }
    }
}

impl Factor for GaussianFactor {
    fn update_message(&mut self, update: Update, variables: &mut Vec<Gaussian>) -> f64 {
        match update {
            Update::ToX => {
                let old_marginal = self.msg_to_x;
                variables[self.idx_x] =  variables[self.idx_x] / self.msg_to_x * self.g;
                self.msg_to_x = self.g;
                abs_diff(&variables[self.idx_x], &old_marginal) 
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