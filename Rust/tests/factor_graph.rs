use TrueChess::ordering::gaussian::*;
use TrueChess::ordering::factorgraph::*;

#[test]
fn gaussian_test() {
    assert_eq!(Gaussian::new(1.0, 2.0).mean(), 0.5);
    let g = Gaussian::new_from_mean_and_variance(1.0, 2.0);
    assert_eq!(g.mean(), 1.0);
    assert_eq!(g.variance(), 2.0);
    assert_eq!(Gaussian::new(1.0, 2.0).variance(), 0.5);

    assert_eq!(abs_diff(&Gaussian::new(0.0, 1.0),&Gaussian::new(0.0, 2.0)), 1.0);
    assert_eq!(abs_diff(&Gaussian::new(0.0, 1.0), &Gaussian::new(0.0, 3.0)), 1.4142135623730951);

    let g1 = Gaussian::new_standard_normal();
    let mut g3 = g1;
    g3 *= g1;

    assert_eq!(abs_diff(&(g1 * g1), &Gaussian::new(0.0, 2.0)), 0.0);
    assert_eq!(abs_diff(&g3, &Gaussian::new(0.0, 2.0)), 0.0);

    let g2 = Gaussian::new(0.0, 0.5);
    let mut g4 = g1;
    g4 /= g2;

    assert_eq!(abs_diff(&(g1 / g2), &Gaussian::new(0.0, 0.5)), 0.0);
    assert_eq!(abs_diff(&g4, &Gaussian::new(0.0, 0.5)), 0.0);

    assert_eq!(log_norm_product(&g1, &g1), -1.2655121234846454);
    assert_eq!(log_norm_ratio(&g1, &g2), 1.612085713764618);
}

#[test]
fn gaussian_factor_tests() {
    let mut fg = FactorGraph::new();
    let s = fg.new_variable();
    let mut f1 = GaussianFactor::new(s, Gaussian::new_from_mean_and_variance(2.0, 42.0));
    let mut f2 = GaussianFactor::new(s, Gaussian::new_from_mean_and_variance(1.0, 1.0));

    assert_eq!(f1.update_message(Update::ToX, &mut fg.variables), 0.1543033499620919);
    assert_eq!(f1.update_message(Update::ToX, &mut fg.variables), 0.0);
    assert_eq!(f2.update_message(Update::ToX, &mut fg.variables), 1.0476190476190477);
    assert_eq!(fg.get_marginal(s).mean(), 1.0232558139534884);
    assert_eq!(fg.get_marginal(s).variance(), 0.9767441860465117);
}