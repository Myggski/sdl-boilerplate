#pragma once

#include <functional>
#include <vector>
#include <algorithm>

namespace Engine
{

  // Helper class to wrap function pointers and member function pointers
  template <typename... Args>
  struct FunctionWrapper
  {
    using EventFunctionType = std::function<void(Args...)>;

    FunctionWrapper(EventFunctionType Function)
        : Id(getNextUniqueId()), Function(Function) {}

    int GetId() const { return Id; }
    EventFunctionType GetFunction() const { return Function; }

  private:
    static int getNextUniqueId()
    {
      static int UniqueId = 0;
      return UniqueId++;
    }

    int Id;
    EventFunctionType Function;
  };

  // GameEvent class to manage events with variable arguments
  template <typename... Args>
  class GameEvent
  {
  public:
    using FunctionType = std::function<void(Args...)>;

    // Add a function to the event (for member functions)
    template <typename T>
    int Add(T *Object, void (T::*MemberFunc)(Args...))
    {
      FunctionWrapper<Args...> FuncWrapper{
          [Object, MemberFunc](Args... args)
          {
            (Object->*MemberFunc)(args...);
          }};
      Functions.push_back(FuncWrapper);
      return FuncWrapper.GetId();
    }

    // Add a lambda or free function to the event
    int Add(const FunctionType &LambdaFunc)
    {
      FunctionWrapper<Args...> FuncWrapper{LambdaFunc};
      Functions.push_back(FuncWrapper);
      return FuncWrapper.GetId();
    }

    // Remove a function from the event
    void Remove(int id)
    {
      Functions.erase(
          std::remove_if(
              Functions.begin(),
              Functions.end(),
              [id](const FunctionWrapper<Args...> &wrapper)
              {
                return wrapper.GetId() == id;
              }),
          Functions.end());
    }

    // Remove all functions from the event
    void RemoveAll()
    {
      Functions.clear();
    }

    // Broadcast the event to all registered functions
    void Broadcast(Args... args)
    {
      Functions.erase(
          std::remove_if(
              Functions.begin(),
              Functions.end(),
              [&](const FunctionWrapper<Args...> &wrapper)
              {
                if (!IsValid(wrapper.GetFunction()))
                {
                  return true; // Erase invalidated function
                }
                else
                {
                  wrapper.GetFunction()(args...);
                  return false;
                }
              }),
          Functions.end());
    }

  private:
    // Check if the function is still valid
    bool IsValid(const FunctionType &Func) const
    {
      return static_cast<bool>(Func);
    }

  private:
    std::vector<FunctionWrapper<Args...>> Functions;
  };

}