/*
DESIGN CONSIDERATION:
--------------------
- Since a keyboard layout is a matrix, a 2D vector is used to represent the layout.

- Dependency Injection has been applied so that this code could be reusable if a different
  chess-piece, sequence length, number of vowels allowed, choice of invalid key and/or 
  layout is to be used.
  
- The valid moves for all the key positions have been calculated beforehand. This enhances 
  the performance multifold since it no longer checks all possible chess-piece moves for each 
  key position during the BFS.
  
- Usage of Recursive approach has been consciously avoided to prevent risk of stack overflow.

- Breadth-first search (BFS) approach was adopted to enhance performance:
    - BFS explores nodes (keys in this case) level by level. This ensures that shorter sequences 
      are explored before longer ones, leading to efficient exploration of the solution space. 
      This can help in quickly identifying valid sequences without unnecessary exploration of deeper levels.
      
    - BFS ensures that each node is visited only once, and all nodes at the same level are visited before
      moving to the next level. This helps in avoiding redundant exploration of the same sequences and 
      prevents revisiting already explored states, leading to efficient utilization of computational resources.
      
    - BFS guarantees that the first valid sequence of the provided length (10 in this case) found from each 
      starting key is also the shortest sequence of its length. This ensures that if a solution exists, 
      BFS will find it in an optimal manner without exploring longer sequences unnecessarily.
      
    - BFS typically requires less memory compared to depth-first search (DFS) because it only needs to store nodes 
      at the current level in the queue. In contrast, DFS can potentially require more memory due to recursion stack 
      space, especially for deep search trees.

ASSUMPTION:
----------
- The last row has two blank places the first and the last column. To specify
  those columns as invalid move the keys at those locations would be considered
  invalid. This remains same even if the blank spaces change coordinates. A suitable
  key (character) is to be chosen to imply invalid position at those coordinates.

TIME COMPLEXITY:
---------------
The time complexity of the GenerateSequences() function depends on the number of
valid sequences that can be generated starting from each key on the keyboard. In
the worst case, each key can generate a maximum of O(8^10) sequences, since there
are 8 possible Knight moves from each key/position and the sequence length is 10.
Therefore, the overall time Complexity is O(N * 8^10), where N is the number of
keys on the keyboard.

SPACE COMPLEXITY:
----------------
In the worst case, if each key generates a maximum number of sequences, 
the space complexity will be O(8^10) to store all sequences. Additionally, 
the space complexity for storing the keyboard layout and other data structures
used in the algorithm is negligible compared to the space required for storing 
the sequences. Therefore, the overall space complexity is O(N * 8^10), where N 
is the number of keys on the keyboard.

OUTPUT:
------
Total number of sequences: 1013398
*/ 

#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <string>
#include <stdexcept>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::pair;
using std::unordered_map;
using std::queue;
using std::invalid_argument;
using std::exception;

using CharVector2D = vector<vector<char>>;
using Coordinates = pair<int, int>;
using ValidKeyMoves = unordered_map<char, vector<Coordinates>>;
using KeySequences = unordered_map<char, vector<string>>;

/*
Creating a base abstract class/interface ChessPiece from which
different child classes could be derived, e.g. Knight, Bishop, Rook.
Since these are specific chess-pieces they would have their own 
specific move coordinates and valid/invalid moves.
*/ 
class ChessPiece
{
public:
    virtual vector<Coordinates> GetMoves() const = 0;
    virtual bool IsValidMove(const int &x, const int &y, const int &moveX, const int &moveY, const char &invalidKey, const CharVector2D& layout) const = 0;
};


/*
Derived a Knight class from the ChessPiece abstract class/interface
specific to this challenge and overriding the base class (interface) 
pure virtual functions specific to Knight moves.
*/ 
class Knight : public ChessPiece 
{
public:
    /* Get the possible moves for a Knight. */
    vector<Coordinates> GetMoves() const override 
    {
        return {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    }

    /* Check if it's valid move. */
    bool IsValidMove(const int &x, const int &y, const int &moveX, const int &moveY, const char &invalidKey, const CharVector2D& layout) const override
    {
        int ROWS = layout.size();
        int COLS = layout[0].size();
        int newX = x + moveX;
        int newY = y + moveY;
        
        /* 
        Check if new position is within the bounds of the keyboard 
        layout and if it's not a null character (empty cell)
        */
        return (newX >= 0) && (newX < ROWS) &&
               (newY >= 0) && (newY < COLS) &&
               /* Checks if the new move ends up on an invalid key */
               (layout[newX][newY] != invalidKey) &&
               /*
               The Knight moves in L shaped motion horizontally and 
               vertically, and in forward a backward direction.
               */ 
               (((1 == abs(moveX)) && (2 == abs(moveY))) ||
                ((2 == abs(moveX)) && (1 == abs(moveY))));
    }
};

/*
The KeyboardLayout class defines the keyboard layout such as the number
of rows and columns available and the keys located at specific coordinates.
This provides the flexibility and re-usability to change layout and keys.
*/ 
class KeyboardLayout 
{
private:
    char invalidKey = 0;
    CharVector2D layout;
public:
    KeyboardLayout(const char &newInvalidKey, const CharVector2D& newLayout) : invalidKey(newInvalidKey), layout(newLayout)
    {
        if (newLayout.empty() || newLayout[0].empty())
        {
            throw invalid_argument("Layout dimensions must be non-zero.");
        }
    }

    const CharVector2D& GetLayout() const 
    {
        return layout;
    }
    
    char GetInvalidKey() const
    {
        return invalidKey;
    }

    int GetRows() const 
    {
        return layout.empty() ? 0 : layout.size();
    }

    int GetCols() const 
    {
        return layout.empty() ? 0 : layout[0].size();
    }
};

/*
The dependencies, i.e. the keyboard layout and the chess-piece are 
injected to the Keyboard class through the constructor hence this class
can be reused if the layout and the chess-piece change.

The Keyboard class verifies if the chess-piece move complies to all 
the constraints and generate unique sequences of the provided length
for each key as a starting position.

It stores unique sequences in an unordered map.

*/
class Keyboard
{
private:
    const unsigned int sequenceLength = 0;
    const unsigned int maxVowelCount = 0;
    const KeyboardLayout& keyboardLayout;
    const ChessPiece* chessPiecePtr;
    ValidKeyMoves keyMoves;
    
    bool IsVowel(char key) 
    {
        const string vowels = "AEIOU";
        return vowels.find(key) != string::npos;
    }
    
    int CountVowels(const string& seq) 
    {
        int count = 0;
        for (char c : seq) 
        {
            if (IsVowel(c)) 
            {
                count++;
            }
        }
        
        return count;
    }
    
    /* 
    Stores all the valid moves (coordinates) against 
    their respective keys in an unordered map.  
    */
    void SetValidMovesForAllKeys()
    {
        const int ROWS = keyboardLayout.GetRows();
        const int COLS = keyboardLayout.GetCols();
        const auto& layout = keyboardLayout.GetLayout();
        char invalidKey = keyboardLayout.GetInvalidKey();
        vector<Coordinates> chessPieceMoves = chessPiecePtr->GetMoves();
        
        for (int i = 0; i < ROWS; i++)
        {
            for (int j = 0; j < COLS; j++)
            {
                char key = layout[i][j];
                vector<Coordinates> moves;
                if(layout[i][j] != invalidKey)
                {
                    for(int k = 0; k < chessPieceMoves.size(); k++)
                    {
                        if(chessPiecePtr->IsValidMove(i, j, chessPieceMoves[k].first, chessPieceMoves[k].second, invalidKey, layout))
                        {
                            moves.push_back({chessPieceMoves[k].first, chessPieceMoves[k].second});
                        }
                    }
                    
                    keyMoves.insert({key, moves});
                }
            }
        }
    }
    
    /* Returns all the valid moves for a particular key. */
    vector<Coordinates> GetValidMovesForAKey(char key) const
    {
        return keyMoves.at(key);
    }

    /* Generate sequences starting from each key on the keyboard */
    KeySequences GenerateSequences() 
    {
        /* 
        Defines an unordered map to store the sequences
        generated for each key on the keyboard. 
        */
        KeySequences sequences;

        const auto& layout = keyboardLayout.GetLayout();
        int ROWS = keyboardLayout.GetRows();
        int COLS = keyboardLayout.GetCols();
        char invalidKey = keyboardLayout.GetInvalidKey();

        /* Iterates through each key on the keyboard */
        for (int i = 0; i < ROWS; ++i)
        {
            for (int j = 0; j < COLS; ++j)
            {
                /* Ignores blank/empty cells/positions */
                if (layout[i][j] != invalidKey) 
                {
                    queue<pair<string, Coordinates>> q;
                    /* Starts with a sequence containing only the starting character */
                    q.push({string(1, layout[i][j]), {i, j}});
                    
                    /* 
                    Initiates a breadth-first search(BFS) search for a key 
                    to explore all possible sequences starting from it.
                    While the queue is not empty:
                      - Pop a sequence (corresponding to a key) and its 
                        latest coordinates and from the queue.
                      - Explore all the valid moves from the current key
                      - For each valid move:
                        - Append the destination key to the sequence.
                        - Add the destination key and the updated coordinates 
                          to the queue for further exploration
                      - Repeat this process until the sequence length reaches the 
                        required length, or there are no more valid moves.
                    */
                    while (!q.empty()) 
                    {
                        auto [seq, pos] = q.front();
                        q.pop();
                        auto [x, y] = pos;
                        
                        /*
                        Checks if vowels count in a sequence exceeds 
                        the provided limit, and if so skips further 
                        exploration from this sequence 
                        */
                        if (CountVowels(seq) > maxVowelCount)
                        {
                            continue;
                        }
                        
                        /*
                        Check if a sequence length reaches the required length, 
                        and if so add it to the map against the key, i.e. the starting 
                        key of the sequence. This applies to all sequences.
                        */ 
                        if (seq.size() == sequenceLength) 
                        {
                            sequences[layout[i][j]].push_back(seq);
                            continue;
                        }
                        
                        /* Loops through the valid moves from the current position */
                        for (const auto& move : GetValidMovesForAKey(layout[x][y])) 
                        {
                            int newX = x + move.first;
                            int newY = y + move.second;
                            q.push({seq + layout[newX][newY], {newX, newY}});
                            
                        }
                    }
                }
            }
        }

        return sequences;
    }

public:
    /* 
    DI: The dependencies: keyboard layout and the chess-piece
    are injected through the constructor. 
    */
    Keyboard(const unsigned int &_sequenceLength, const unsigned int &_maxVowelCount, const KeyboardLayout& _keyboardLayout, const ChessPiece* _chessPiecePtr)
        : sequenceLength(_sequenceLength), maxVowelCount(_maxVowelCount), keyboardLayout(_keyboardLayout), chessPiecePtr(_chessPiecePtr) 
    {
        try
        {
            if(0 == sequenceLength)
            {
                throw invalid_argument("Sequence length must be non-zero.");
            }
            
            SetValidMovesForAllKeys();
        } 
        catch (const exception& e) 
        {
            cerr << "Exception during Keyboard initialization: " << e.what() << endl;
            throw;
        }
    }

    /*
    Displays the total number of unique sequences 
    possible with the provided constraints.
    */ 
    void displayTotalSequences() 
    {
        try
        {
            int totalSequenceCout = 0;
            auto sequences = GenerateSequences();
            for (const auto& seq : sequences) 
            {
                totalSequenceCout += seq.second.size();
            }
    
            cout << "Total number of sequences: " << totalSequenceCout << endl;
        }
        catch (const exception& e) 
        {
            cerr << "Exception during sequence generation: " << e.what() << endl;
            throw;
        }
        
    }
};

int main() 
{
    char invalidKey = 0;
    CharVector2D layout = {
        {'A', 'B', 'C', 'D', 'E'},
        {'F', 'G', 'H', 'I', 'J'},
        {'K', 'L', 'M', 'N', 'O'},
        {invalidKey, '1', '2', '3', invalidKey}
    };

    try
    {
        KeyboardLayout keyboardLayout(invalidKey, layout);
        Knight knight;
        Keyboard keyboard(10, 2, keyboardLayout, &knight);
    
        keyboard.displayTotalSequences();
    }
    catch (const exception& e) 
    {
        cerr << "Program execution failed. Terminating..." << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
