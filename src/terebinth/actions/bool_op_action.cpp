#include "../action.h"
#include "../error_handler.h"
#include "../string_drawing.h"

class AndAction: public ActionData {
public:
  AndAction(Action first_action_in, Action second_action_in)
    : ActionData(Bool, Void, Void) {
    first_action_ = first_action_in;
    second_action_ = second_action_in;

    if (first_action_->GetReturnType() != Bool) {
      throw TerebinthError("AndAction created with first action that does not return Bool", INTERNAL_ERROR);
    }

    if (second_action_->GetReturnType() != Bool) {
      throw TerebinthError("AndAction created with second action that does not return Bool", INTERNAL_ERROR);
    }
  }

  std::string GetDescription() {
    return str::MakeRootUpBinaryTree("&&", first_action_->GetReturnType()->GetName(),
        second_action_->GetReturnType()->GetName(), first_action_->GetDescription(),
        second_action_->GetDescription());
  }

  void* Execute(void* in_left, void* in_right) {
    bool* out = (bool*)malloc(sizeof(bool));
    *out = false;
    void* first_val = first_action_->Execute(nullptr, nullptr);

    if (*((bool*)first_val)) {
      void* second_val = second_action_->Execute(nullptr, nullptr);

      if (*((bool*)second_val)) {
        *out = true;
      }

      free(second_val);
    }
    free(first_val);

    return out;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram* prog) {
    prog->PushExpr();
      first_action_->AddToProg(void_action_, void_action_, prog);
    prog->PopExpr();

    prog->Code(" && ");

    prog->PushExpr();
      second_action_->AddToProg(void_action_, void_action_, prog);
    prog->PopExpr();
  }

private:
  Action first_action_;
  Action second_action_;
};

class OrAction: public ActionData
{
public:	
	OrAction(Action first_action_in, Action second_action_in)
		:ActionData(Bool, Void, Void)
	{
		first_action = first_action_in;
		second_action = second_action_in;
		
		if (first_action->GetReturnType() != Bool)
		{
			throw TerebinthError("OrAction created with first action that does not return Bool", INTERNAL_ERROR);
		}
		
		if (second_action->GetReturnType() != Bool)
		{
			throw TerebinthError("OrAction created with second action that does not return Bool", INTERNAL_ERROR);
		}
	}

  std::string GetDescription() {
		return str::MakeRootUpBinaryTree("||", first_action->GetReturnType()->GetName(),
        second_action->GetReturnType()->GetName(), first_action->GetDescription(), second_action->GetDescription());
	}
	
	void* Execute(void* inLeft, void* inRight) {
		bool* out = (bool*)malloc(sizeof(bool));
		*out = true;
		void* first_val = first_action->Execute(nullptr, nullptr);
		
		if (!*((bool*)first_val))
		{
			void* second_val = second_action->Execute(nullptr, nullptr);
			
			if (!*((bool*)second_val))
			{
				*out = false;
			}
			
			free(second_val);
		}
		
		free(first_val);
		
		return out;
	}
	
	void addToProg(Action in_left, Action in_right, CppProgram* prog)
	{
		prog->PushExpr();
			first_action->AddToProg(void_action_, void_action_, prog);
		prog->PopExpr();
		
		prog->Code(" || ");
		
		prog->PushExpr();
			second_action->AddToProg(void_action_, void_action_, prog);
		prog->PopExpr();
	}
	
private:
	
	Action first_action;
	Action second_action;
};

Action AndAction(Action first_action_in, Action second_action_in)
{
	return Action(AndAction(first_action_in, second_action_in));
}

Action OrAction(Action first_action_in, Action second_action_in)
{
	return Action(OrAction(first_action_in, second_action_in));
}
