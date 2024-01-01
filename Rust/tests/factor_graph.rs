use TrueChess::ordering::gaussian::Gaussian;

#[test]
fn gaussian_test() {
    assert_eq!(Gaussian::new(1.0, 2.0).mean(), 0.5);

    let g = Gaussian::new_from_mean_and_variance(1.0, 2.0);
    assert_eq!(g.mean(), 1.0);
    assert_eq!(g.variance(), 2.0);

    assert_eq!(Gaussian::new(1.0, 2.0).variance(), 0.5);
    assert_eq!(Gaussian::new(0.0, 1.0).abs_diff(&Gaussian::new(0.0, 2.0)), 1.0);
    assert_eq!(Gaussian::new(0.0, 1.0).abs_diff(&Gaussian::new(0.0, 3.0)), 1.4142135623730951);
    
}