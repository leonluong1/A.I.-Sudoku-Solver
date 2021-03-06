#include"BTSolver.hpp"
#include <set>

using namespace std;

// =====================================================================
// Constructors
// =====================================================================

BTSolver::BTSolver ( SudokuBoard input, Trail* _trail,  string val_sh, string var_sh, string cc )
: sudokuGrid( input.get_p(), input.get_q(), input.get_board() ), network( input )
{
	valHeuristics = val_sh;
	varHeuristics = var_sh;
	cChecks =  cc;

	trail = _trail;
}

// =====================================================================
// Consistency Checks
// =====================================================================

// Basic consistency check, no propagation done
bool BTSolver::assignmentsCheck ( void )
{
	for ( Constraint c : network.getConstraints() )
		if ( ! c.isConsistent() )
			return false;

	return true;
}

/**
 * Part 1 TODO: Implement the Forward Checking Heuristic
 *
 * This function will do both Constraint Propagation and check
 * the consistency of the network
 *
 * (1) If a variable is assigned then eliminate that value from
 *     the square's neighbors.
 *
 * Note: remember to trail.push variables before you assign them
 * Return: true is assignment is consistent, false otherwise
 */
bool BTSolver::forwardChecking ( void )
{
    for (Constraint* c : network.getModifiedConstraints() )
		for (Variable* v : c->vars)
			if (v->isAssigned())
                for (Variable* cv : c->vars)
                    if (v != cv && cv->getDomain().contains(v->getAssignment()))
                    {
                        trail->push(cv);
                        cv->removeValueFromDomain(v->getAssignment());
                        if (cv->size() == 0)
                            return false;
                    }
	return true;

}

/**
 * Part 2 TODO: Implement both of Norvig's Heuristics
 *
 * This function will do both Constraint Propagation and check
 * the consistency of the network
 *
 * (1) If a variable is assigned then eliminate that value from
 *     the square's neighbors.
 *
 * (2) If a constraint has only one possible place for a value
 *     then put the value there.
 *
 * Note: remember to trail.push variables before you assign them
 * Return: true is assignment is consistent, false otherwise
 */
bool BTSolver::norvigCheck ( void )
{
    deque<Variable*> q;
    set<Variable*> s;
    for (Constraint* c : network.getModifiedConstraints() )
        for (Variable* v : c->vars)
            if (v->isAssigned())
                for (Variable* cv : c->vars)
                {
                    if (v != cv && cv->getDomain().contains(v->getAssignment()))
                    {
                        trail->push(cv);
                        cv->removeValueFromDomain(v->getAssignment());
                        if (cv->size() == 0)
                            return false;
                        if (cv->isAssigned() && s.find(cv) == s.end())
                        {
                            q.push_back(cv);
                            s.insert(cv);
                        }
                    }
                }
    Variable* va;
    while ( !q.empty() )
    {
        va = q.front();
        s.erase(va);
        q.pop_front();
        for (Variable* nv : network.getNeighborsOfVariable(va))
            if (nv->getDomain().contains(va->getAssignment()))
            {
                trail->push(nv);
                nv->removeValueFromDomain(va->getAssignment());
                if (nv->size() == 0)
                    return false;
                if (nv->isAssigned() && s.find(nv) == s.end())
                {
                    q.push_back(nv);
                    s.insert(nv);
                }
            }
    }
    return true;
}

/**
 * Optional TODO: Implement your own advanced Constraint Propagation
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
bool BTSolver::getTournCC ( void )
{
	return false;
}

// =====================================================================
// Variable Selectors
// =====================================================================

// Basic variable selector, returns first unassigned variable
Variable* BTSolver::getfirstUnassignedVariable ( void )
{
	for ( Variable* v : network.getVariables() )
		if ( !(v->isAssigned()) )
			return v;

	// Everything is assigned
	return nullptr;
}

/**
 * Part 1 TODO: Implement the Minimum Remaining Value Heuristic
 *
 * Return: The unassigned variable with the smallest domain
 */
Variable* BTSolver::getMRV ( void )
{
	Variable* mvar = getfirstUnassignedVariable();
	if ( !mvar )
		return nullptr;

	int min = mvar->size();
	for ( Variable* v: network.getVariables() )
		if ( v->size() < min && !(v->isAssigned()) )
		{
			min = v->size();
			mvar = v;
		}

	return mvar;

}

/**
 * Part 2 TODO: Implement the Degree Heuristic
 *
 * Return: The unassigned variable with the most unassigned neighbors
 */
Variable* BTSolver::getDegree ( void )
{
	Variable* mvar = getfirstUnassignedVariable();
    if (!mvar )
        return nullptr;

    int max = 0;
    for ( Variable* v : network.getNeighborsOfVariable(mvar) )
        if (!(v->isAssigned()))
            max += 1;

    for (Variable* va : network.getVariables())
    {
        if (!(va->isAssigned()))
        {
            int count = 0;
            for ( Variable* nv : network.getNeighborsOfVariable(va) )
                if (!(nv->isAssigned()))
                    count += 1;
            if (count > max)
            {
                max = count;
                mvar = va;
            }
        }
    }
    return mvar;
}

/**
 * Part 2 TODO: Implement the Minimum Remaining Value Heuristic
 *                with Degree Heuristic as a Tie Breaker
 *
 * Return: The unassigned variable with the smallest domain and involved
 *             in the most constraints
 */
Variable* BTSolver::MRVwithTieBreaker ( void )
{
    Variable* fv = getfirstUnassignedVariable();
    if ( !fv )
        return nullptr;

    int min = fv->size();
    for ( Variable* v: network.getVariables() )
        if ( v->size() < min && !(v->isAssigned()) )
            min = v->size();

    ConstraintNetwork::VariableSet s;
    for ( Variable* v: network.getVariables() )
        if (v->size() == min)
            s.push_back( v );

    Variable* mvar = s[0];
    int max = 0;
    for ( Variable* nv : network.getNeighborsOfVariable(mvar) )
        if (!(nv->isAssigned()))
            max += 1;

    for (Variable* sv : s)
    {
        int count = 0;
        for ( Variable* nv : network.getNeighborsOfVariable(sv) )
            if (!(nv->isAssigned()))
                count += 1;
        if (count > max)
        {
            max = count;
            mvar = sv;
        }

    }
    return mvar;
}

/**
 * Optional TODO: Implement your own advanced Variable Heuristic
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
Variable* BTSolver::getTournVar ( void )
{
	return nullptr;
}

// =====================================================================
// Value Selectors
// =====================================================================

// Default Value Ordering
vector<int> BTSolver::getValuesInOrder ( Variable* v )
{
	vector<int> values = v->getDomain().getValues();
	sort( values.begin(), values.end() );
	return values;
}

/**
 * Part 1 TODO: Implement the Least Constraining Value Heuristic
 *
 * The Least constraining value is the one that will knock the least
 * values out of it's neighbors domain.
 *
 * Return: A list of v's domain sorted by the LCV heuristic
 *         The LCV is first and the MCV is last
 */
vector<int> BTSolver::getValuesLCVOrder ( Variable* v )
{
	vector<pair<int, int>> all;
    vector<int> lcv;

    for (int val : v->getDomain())
    {
        pair<int, int> p;
        p.first = val;
        p.second = 0;

        for ( Variable* neigh : network.getNeighborsOfVariable(v) )
            if ( neigh->getDomain().contains(val) )
                p.second += 1;

        all.push_back(p);
    }

    sort(all.begin(), all.end(), [](pair<int, int> a, pair<int, int> b){return a.second < b.second;});

    for (pair<int, int> p : all)
        lcv.push_back(p.first);

    return lcv;
}

/**
 * Optional TODO: Implement your own advanced Value Heuristic
 *
 * Completing the three tourn heuristic will automatically enter
 * your program into a tournament.
 */
vector<int> BTSolver::getTournVal ( Variable* v )
{
	return vector<int>();
}

// =====================================================================
// Engine Functions
// =====================================================================

void BTSolver::solve ( void )
{
	if ( hasSolution )
		return;

	// Variable Selection
	Variable* v = selectNextVariable();

	if ( v == nullptr )
	{
		for ( Variable* var : network.getVariables() )
		{
			// If all variables haven't been assigned
			if ( ! ( var->isAssigned() ) )
			{
				cout << "Error" << endl;
				return;
			}
		}

		// Success
		hasSolution = true;
		return;
	}

	// Attempt to assign a value
	for ( int i : getNextValues( v ) )
	{
		// Store place in trail and push variable's state on trail
		trail->placeTrailMarker();
		trail->push( v );

		// Assign the value
		v->assignValue( i );

		// Propagate constraints, check consistency, recurse
		if ( checkConsistency() )
			solve();

		// If this assignment succeeded, return
		if ( hasSolution )
			return;

		// Otherwise backtrack
		trail->undo();
	}
}

bool BTSolver::checkConsistency ( void )
{
	if ( cChecks == "forwardChecking" )
		return forwardChecking();

	if ( cChecks == "norvigCheck" )
		return norvigCheck();

	if ( cChecks == "tournCC" )
		return getTournCC();

	return assignmentsCheck();
}

Variable* BTSolver::selectNextVariable ( void )
{
	if ( varHeuristics == "MinimumRemainingValue" )
		return getMRV();

	if ( varHeuristics == "Degree" )
		return getDegree();

	if ( varHeuristics == "MRVwithTieBreaker" )
		return MRVwithTieBreaker();

	if ( varHeuristics == "tournVar" )
		return getTournVar();

	return getfirstUnassignedVariable();
}

vector<int> BTSolver::getNextValues ( Variable* v )
{
	if ( valHeuristics == "LeastConstrainingValue" )
		return getValuesLCVOrder( v );

	if ( valHeuristics == "tournVal" )
		return getTournVal( v );

	return getValuesInOrder( v );
}

bool BTSolver::haveSolution ( void )
{
	return hasSolution;
}

SudokuBoard BTSolver::getSolution ( void )
{
	return network.toSudokuBoard ( sudokuGrid.get_p(), sudokuGrid.get_q() );
}

ConstraintNetwork BTSolver::getNetwork ( void )
{
	return network;
}
