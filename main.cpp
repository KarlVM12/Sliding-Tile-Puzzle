#include <iostream>
#include <queue>
#include <map>
#include <algorithm>
#include <functional>

// Node struct
struct Node
{
    // state represented by an int array where (wwww0bbbb) is equivalent to {1,1,1,1,0,-1,-1,-1,-1}
    std::vector<int> state;
    const Node * parent;
    std::pair<int, bool> prevAction;
    int pathCost;
    int h, f;

    // default h1(n) = 4, h2(n) = 8
    Node(std::vector<int> v = {1,1,1,1,0,-1,-1,-1,-1}, Node * p = nullptr, std::pair<int, bool> pA = {0, false}, int pC = 0, int hn = 4, int fn = 0) : state(v), parent(p), prevAction(pA), pathCost(pC), h(hn), f(fn) {}

    // copy constructor
    Node(const Node * n)
    {
        state = n->state;
        parent = n->parent;
        prevAction = n->prevAction;
        pathCost = n->pathCost;
        h = n->h;
        f = n->f;
    }
    
};

// comparison for two Nodes
bool operator==(const Node& lhs, const Node& rhs)
{
    // if same state, considered equal
    if(lhs.state == rhs.state)
        return true;

    return false;
}

// output stream for states of nodes
std::ostream& operator<<(std::ostream& os, const Node * n)
{   
    os << "{ ";
    for(size_t i = 0; i < n->state.size(); ++i)
    {
       if(n->state[i] == 1)
            os << "w" << " ";
        else if (n->state[i] == -1)
            os << "b" << " ";
        else
            os << "0" << " ";
    }
    os << "} ";

    return os;
}

// Problem struct
struct Problem
{
    Node * initial = new Node();
    Node * goal = new Node({-1,-1,-1,-1,0,1,1,1,1});

    // which heuristic function to use, defaults to true = h1(n), false = h2(n);
    bool h1ORh2;
    
    std::vector<int> actionCost = {1,2,3,4};

    int numOfNodesWithChildren = 0;
    int numOfNodesLeftInFrontier = 0;
    int totalNumOfNodesInFrontier = 0;

    Problem(Node * initialIn = new Node(), Node * goalIn = new Node({-1,-1,-1,-1,0,1,1,1,1}), bool h1ORh2In = true) : initial(initialIn), goal(goalIn), h1ORh2(h1ORh2In) 
    {
        initial->h = h1ORh2 ? 4 : 8;
    }

    // Since each state can follow with a max of 8 possible actions,
    // use LorR to determine if we are swapping to the left or to the right
    // true = left, false = right
    Node * result(const Node * s, int action, bool LorR)
    {
        Node * child = new Node(s);
        
        // finds the location of the space to swap with a tile
        size_t spaceLocation = findSpace(child->state);


        if (LorR) 
        {
            // {1,1,1,1,0,-1,-1,-1,-1} ==> {1,1,1,0,1,-1,-1,-1,-1}, space goes left how many actions
            std::swap(child->state[spaceLocation - action], child->state[spaceLocation]);
        }
        else
        {
            // {1,1,1,1,0,-1,-1,-1,-1} ==> {1,1,1,,1,-1,0,-1,-1,-1}, space goes right how many actions     
            std::swap(child->state[spaceLocation + action], child->state[spaceLocation]);
        }

        // reassigns new child with proper data, calculate remaining h and pathcost
        child->parent = s;
        child->pathCost = s->pathCost + action;
        child->prevAction = {action, LorR};
        child->h = calculateHn(child->state);

        return child;
    
    } // end result()

    // returns the index of the space in the state vector, 
    // else return one greater than the vectors size
    size_t findSpace(std::vector<int> state)
    {
        for(size_t i = 0; i < state.size(); ++i)
            if(state[i] == 0)
                return i;

        return state.size();
    
    } // end findSpace()

    // heuristic function calculation
    int calculateHn(std::vector<int> state)
    {
        
        if(h1ORh2)
        {
            // h1(n) = how many 1s (Ws) are still left of -1 (Bs), starts at h = 4
            int left1s = 0;
            for(int i = 0; i < 5; ++i)
                if(state[i] == 1)
                    ++left1s;

            return left1s;  
        }
        else
        {
            // h2(n) = tiles missplaced from goal state, starts at h = 8
            int missplaced = 0;
            for (size_t i = 0; i < goal->state.size(); ++i)
                if (goal->state[i] != state[i])
                    ++missplaced;

            return missplaced;
        } 

    } // end calculateHn()
};

// Expands the passed node into its possible successors
std::vector<Node*> expand(Problem &problem, Node * node)
{
    Node * s = new Node(node);
    std::vector<Node*> children;

    // depending on space, {4,5,6,7,8,7,6,5,4} max possible swaps from that index
    size_t spaceIndex = problem.findSpace(s->state);

    // left
    // {0,1,2,3,4,4,4,4,4} max possible swaps left from that index
    for(size_t i = 0; i < ((spaceIndex < 4) ? spaceIndex : problem.actionCost.size()); ++i)
    {   
        if(spaceIndex == 0)
            break;

        children.push_back(problem.result(s, problem.actionCost[i], true));
    }

    // right
    // {4,4,4,4,4,3,2,1,0} possible swaps right from that index
    for(size_t i = 0; i < ((spaceIndex > 4) ? spaceIndex - (2 * (spaceIndex-4)) : problem.actionCost.size()); ++i)
    {
        if(spaceIndex == s->state.size()-1)
            break;

        children.push_back(problem.result(s, problem.actionCost[i], false));
    }
 
    // returns children of node
    return children;

} // end expand()

// orders priority queue frontier by f, with lowest f first
struct orderf {
    constexpr bool operator()( std::pair<Node*, int> const& a, std::pair<Node*, int> const& b) const noexcept
    {
        return a.second > b.second;
    }
};

// deletes remaining data in frontier after goal state reached
void clearFrontier(std::priority_queue<std::pair<Node*, int>, std::vector<std::pair<Node*, int>>, orderf> &frontier)
{
    while(!frontier.empty())
    {
        Node * temp = frontier.top().first;
        frontier.pop();
        delete temp;
    }

} // end clearFrontier()

// A* search with BestFirstSearch, the passed f = g+ h
Node * BestFirstSearch(Problem &problem, std::function<int(int,int)> f)
{
    Node * node = problem.initial; // initial state;
    
    // ordered by f, with node as the first element
    std::priority_queue<std::pair<Node*, int>, std::vector<std::pair<Node*, int>>, orderf> frontier; 
    frontier.push(std::make_pair(node, 0));

    // keeps track of amount of nodes
    problem.totalNumOfNodesInFrontier = 1;
    problem.numOfNodesLeftInFrontier = 1;

    // map with reached node state and its cost
    std::map<std::vector<int>, int> reached;
    reached.insert({node->state, 0});


    while (!frontier.empty())
    {
        node = frontier.top().first;
        frontier.pop();
        problem.numOfNodesLeftInFrontier -= 1;
    
        // if new node is goal, return state
        if (node == problem.goal || node->h == 0)
        { 
            reached.clear();
            clearFrontier(frontier);
            return node;
        }
        // else

        // expand parent to see the next frontier
        std::vector<Node*> children = expand(problem, node);
        problem.numOfNodesWithChildren += 1;

        // frees memory of parent
        delete node;

        // checks each child of parent and evaulates them based on f
        for(size_t i = 0; i < children.size(); ++i)
        {
            // evaluation function, g + h
            int fn = f(children[i]->pathCost, children[i]->h);
            
            // If find reached the end of reached, node is not in reached so explore that frontier 
            // or if current path cost of child is less than the one found in reached, update reached and add to frontier
            if(reached.find(children[i]->state) == reached.end() || fn < reached[children[i]->state] )
            {
                // if already in reached, reassigns node path with a better cost
                reached.insert_or_assign(children[i]->state, fn);
                frontier.push(std::make_pair(children[i], fn));
                children[i]->f = fn;
                
                problem.totalNumOfNodesInFrontier +=1;
                problem.numOfNodesLeftInFrontier +=1;
            }
            else
            {
                // if child doesn't enter frontier, frees up memory
                delete children[i];
            }
        
        } // end for

    } // end while


    // returns nullptr if fails
    return nullptr;

} // end BestFirstSearch()

// prints data from problem
void printProblemInfo(const Node * solution, const Problem problem)
{
    std::cout << "--------------------[Finished (using h" << (problem.h1ORh2 ? 1 : 2) << "(n))]-----------------------" << std::endl;
    std::cout << "Goal found: " << solution << " (b = black, 0 = space, w = white)"<<std::endl;
    std::cout << "h(GOAL) = " << solution->h << std::endl;
    std::cout << "f(GOAL) = " << solution->f << std::endl;
    std::cout << "Number of nodes that expanded successors: " << problem.numOfNodesWithChildren << std::endl;
    std::cout << "Number of nodes remaining in frontier list after goal reached: " << problem.numOfNodesLeftInFrontier << std::endl;
    std::cout << "Total number of nodes ever in frontier: " << problem.totalNumOfNodesInFrontier << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;

} // end printProblemInfo()

// prints solution sequence
void printSolutionSequence(const Node * solution)
{
    const Node * cursor = solution;
    std::cout << cursor <<  " (" << cursor->prevAction.first  << ", "<< (cursor->prevAction.second ? "left" : "right") << ") g:" << cursor->pathCost << " h:"  << cursor->h << " f:" << cursor->f << " (Goal State!)"<< std::endl;
    std::cout << "          /\\        " << std::endl;
    cursor = cursor->parent;

    while(cursor->parent != nullptr)
    {
        std::cout << cursor <<  " (" << cursor->prevAction.first  << ", "<< (cursor->prevAction.second ? "left" : "right") << ") g:" << cursor->pathCost << " h:"  << cursor->h << " f:" << cursor->f << std::endl;
        std::cout << "          /\\        " << std::endl;

        cursor = cursor->parent;
    } 
    std::cout << cursor <<  " (" << cursor->prevAction.first  << ", "<< (cursor->prevAction.second ? "left" : "right") << ") g:" << cursor->pathCost << " h:"  << cursor->h << " f:" << cursor->f << " (Initial State)" << std::endl;
    
} // end printSolutionSequence()


int main()
{
    // Sliding tiles puzzle using h1(n) = the amount of Ws left of Bs
    Problem problem(new Node({1,1,1,1,0,-1,-1,-1,-1}), new Node({-1,-1,-1,-1,0,1,1,1,1}), true);
    std::function<int(int, int)> f = [](int g, int h) -> int { return g + h; };

    Node * solutionh1 = BestFirstSearch(problem, f);
    
    printProblemInfo(solutionh1, problem);
    printSolutionSequence(solutionh1);  

    std::cout << "===================================================================" << std::endl;
    std::cout << "===================================================================" << std::endl;

    // Sliding tiles puzzle using h2(n) = the amount of tiles missplaced from goal state
    problem = Problem(new Node({1,1,1,1,0,-1,-1,-1,-1}), new Node({-1,-1,-1,-1,0,1,1,1,1}), false);
    
    Node * solutionh2 = BestFirstSearch(problem, f);

    printProblemInfo(solutionh2, problem);
    printSolutionSequence(solutionh2); 

    return 0;
}
