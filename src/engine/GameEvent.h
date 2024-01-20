#include <functional>
#include <vector>
#include <algorithm>

template <typename... Args>
struct FunctionWrapper
{
    using FunctionType = std::function<void(Args...)>;

public:
    FunctionWrapper(FunctionType function)
        : id(get_unique_id()),
          function(function) {}

    int get_id() const { return id; };
    FunctionType get_function() const { return function; };

private:
    static int get_unique_id()
    {
        static int unique_id{0};
        return unique_id++;
    }

    int id;
    FunctionType function;
};

template <typename... Args>
class GameEvent
{
public:
    using FunctionType = std::function<void(Args...)>;

    // Add a function to the event
    template <typename T>
    int add(T *object, void (T::*memberFunction)(Args...))
    {
        FunctionWrapper<Args...> function_wrapper{
            [object, memberFunction](Args... args)
            { (object->*memberFunction)(args...); }};

        functions.push_back(function_wrapper);

        return function_wrapper.get_id();
    }

    // Add a lambda or free function to the event
    int add(const FunctionType &lambda_func)
    {
        FunctionWrapper<Args...> function_wrapper{lambda_func};
        functions.push_back(function_wrapper);

        return function_wrapper.get_id();
    }

    // Remove a function from the event
    void remove(const int id)
    {
        functions.erase(
            std::remove_if(
                functions.begin(),
                functions.end(),
                [&id](const FunctionWrapper<Args...> &wrapper)
                {
                    return wrapper.get_id() == id;
                }),
            functions.end());
    }

    void remove_all()
    {
        functions.clear();
    }

    // Broadcast the event to all registered functions
    void broadcast(Args... args)
    {
        std::erase_if(
            functions,
            [=](const FunctionWrapper<Args...> &wrapper)
            {
                if (!is_valid(wrapper.get_function()))
                {
                    return true; // Erase invalidated function
                }
                else
                {
                    wrapper.get_function()(args...);
                    return false;
                }
            });
    }

private:
    // Check if the function is still valid
    bool is_valid(const FunctionType &function) const
    {
        return static_cast<bool>(function);
    }

private:
    std::vector<FunctionWrapper<Args...>> functions;
};