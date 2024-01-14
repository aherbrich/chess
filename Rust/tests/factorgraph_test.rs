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
    let mut db = DistributionBag::new();
    let s = db.new_variable();
    let mut f1 = GaussianFactor::new(s, Gaussian::new_from_mean_and_variance(2.0, 42.0));
    let mut f2 = GaussianFactor::new(s, Gaussian::new_from_mean_and_variance(1.0, 1.0));

    assert_eq!(f1.update_message(Update::ToX, &mut db.variables), 0.1543033499620919);
    assert_eq!(f1.update_message(Update::ToX, &mut db.variables), 0.0);
    assert_eq!(f2.update_message(Update::ToX, &mut db.variables), 1.0);
    assert_eq!(db.get_marginal(s).mean(), 1.0232558139534884);
    assert_eq!(db.get_marginal(s).variance(), 0.9767441860465117);
}

#[test]
fn gaussian_mean_factor_tests() {
    let mut db = DistributionBag::new();
    let s1 = db.new_variable();
    let s2 = db.new_variable();
    let mut f = GaussianFactor::new(s1, Gaussian::new_from_mean_and_variance(3.0, 1.0));
    let mut g = GaussianMeanFactor::new(s2, s1, 0.5);

    assert_eq!(f.update_message(Update::ToX, &mut db.variables), 3.0);
    assert_eq!(g.update_message(Update::ToX, &mut db.variables), 2.0);
    assert_eq!(db.get_marginal(s1).mean(), 3.0);
    assert_eq!(db.get_marginal(s2).mean(), 3.0);
    assert_eq!(db.get_marginal(s1).variance(), 1.0);
    assert_eq!(db.get_marginal(s2).variance(), 1.5);
}

#[test]
fn weighted_sum_factor_tests() {
    let mut db = DistributionBag::new();
    let s1 = db.new_variable();
    let s2 = db.new_variable();
    let s3 = db.new_variable();
    let mut f1 = GaussianFactor::new(s1, Gaussian::new_from_mean_and_variance(1.0, 1.0));
    let mut f2 = GaussianFactor::new(s2, Gaussian::new_from_mean_and_variance(2.0, 4.0));
    let mut f3 = GaussianFactor::new(s3, Gaussian::new_from_mean_and_variance(2.0, 0.5));
    let mut g = WeightedSumFactor::new(s1, s2, s3, 0.5, 0.5);

    assert_eq!(f1.update_message(Update::ToX, &mut db.variables), 1.0);
    assert_eq!(f2.update_message(Update::ToX, &mut db.variables), 0.5);
    assert_eq!(g.update_message(Update::ToZ, &mut db.variables), 1.2);
    assert!((db.get_marginal(s3).mean() - 1.5).abs() < 1e-6);
    assert_eq!(db.get_marginal(s3).variance(), 1.25);

    assert_eq!(f3.update_message(Update::ToX, &mut db.variables), 4.0);
    assert_eq!(db.get_marginal(s3).mean(), 1.8571428571428574);
    assert_eq!(db.get_marginal(s3).variance(), 0.35714285714285715);

    assert_eq!(g.update_message(Update::ToX, &mut db.variables), 0.40824829046386313);
    assert_eq!(db.get_marginal(s1).mean(), 1.142857142857143);
    assert_eq!(db.get_marginal(s1).variance(), 0.8571428571428571);

    assert!((g.update_message(Update::ToY, &mut db.variables) - 1.0) < 1e-6);
    assert_eq!(db.get_marginal(s2).mean(), 2.571428571428572);
    assert_eq!(db.get_marginal(s2).variance(), 1.7142857142857144);
}


#[test]
fn greater_than_factor_tests() {
    let mut db = DistributionBag::new();
    let s = db.new_variable();
    let mut f = GaussianFactor::new(s, Gaussian::new_from_mean_and_variance(1.0, 1.0));
    let mut g = GreaterThanFactor::new(s, 0.0);

    assert_eq!(f.update_message(Update::ToX, &mut db.variables), 1.0);
    assert_eq!(g.update_message(Update::ToX, &mut db.variables), 1.0448277182202785);
    assert_eq!(db.get_marginal(s).mean(), 1.2875999709391783);
    assert_eq!(db.get_marginal(s).variance(), 0.6296862857766055);
}

#[test]
fn ranking_graph_tests() {
    // define the factor graph
    let mut db = DistributionBag::new();
    let urgency = vec![db.new_variable(), db.new_variable(), db.new_variable()];
    let latent_urgency = vec![db.new_variable(), db.new_variable(), db.new_variable()];
    let diffs = vec![db.new_variable(), db.new_variable()];
    let mut prior_factor = (0..urgency.len())
        .map(|i| 
            GaussianFactor::new(urgency[i], Gaussian::new_from_mean_and_variance(0.0, 1.0)))
        .collect::<Vec<_>>();
    let mut latent_factor = (0..latent_urgency.len())
        .map(|i| 
            GaussianMeanFactor::new(latent_urgency[i], urgency[i], 1.0/2.0 * 1.0/2.0))
        .collect::<Vec<_>>();
    let mut diff_factor = (0..diffs.len())
        .map(|i| 
            WeightedSumFactor::new(latent_urgency[0], latent_urgency[i+1], diffs[i], 1.0, -1.0)
        )
        .collect::<Vec<_>>();
    let mut greater_than_factor = (0..diffs.len())
        .map(|i| 
            GreaterThanFactor::new(diffs[i], 0.0)
        )
        .collect::<Vec<_>>();

    // message passing schedule
    for i in 0..urgency.len() {
        prior_factor[i].update_message(Update::ToX, &mut db.variables);
        latent_factor[i].update_message(Update::ToX, &mut db.variables);
    }

    let mut delta : f64 = 1e4;
    while delta > 1e-6 {
        delta = 0.0;

        for i in 0..diffs.len() {
            delta = delta.max(diff_factor[i].update_message(Update::ToZ, &mut db.variables));
            delta = delta.max(greater_than_factor[i].update_message(Update::ToX, &mut db.variables));
            delta = delta.max(diff_factor[i].update_message(Update::ToX, &mut db.variables));
            delta = delta.max(diff_factor[i].update_message(Update::ToY, &mut db.variables));
        }
    }

    for i in 0..urgency.len() {
        latent_factor[i].update_message(Update::ToY, &mut db.variables);
    }

    // check the results
    assert_eq!(db.get_marginal(urgency[0]).mean(), 0.7563395960508632);
    assert_eq!(db.get_marginal(urgency[1]).mean(), -0.3781697980254337);
    assert_eq!(db.get_marginal(urgency[2]).mean(), -0.378169798025432);
    assert_eq!(db.get_marginal(urgency[0]).variance(), 0.6429275764834135);
    assert_eq!(db.get_marginal(urgency[1]).variance(), 0.7494990233425036);
    assert_eq!(db.get_marginal(urgency[2]).variance(), 0.7494990233425054);

    // print the results
    println!("\n\nThree Move example\n=================");
    for i in 0..urgency.len() {
        println!("urgency[{0}] = {1} +/- {2}", i, 
            db.get_marginal(urgency[i]).mean(), 
            db.get_marginal(urgency[i]).variance());
    }
}