var CreateAccountDialog =
{
    getCreateAccountText : function()
    {
        return qsTr("Create Account");
    },
    getUsernameText : function()
    {
        return qsTr("Username:");
    },
    getPasswordText : function()
    {
        return qsTr("Password:");
    },
    getPasswordHelpText : function()
    {
        return qsTr("Password of your account. Needs to contain 8 letters, one capital one small letter a number and a special char.");
    },
    getRepeatPasswordText : function()
    {
        return qsTr("Repeat password:");
    },
    getMailText : function()
    {
        return qsTr("E-Mail:");
    },
    getMailHelpText : function()
    {
        return qsTr("E-Mail-Address for your account.");
    },
    getAbortText : function()
    {
        return qsTr("Abort");
    },
    getAbortHelpText : function()
    {
        return qsTr("Aborts the new account creation on the server.");
    },
    getCreateText : function()
    {
        return qsTr("Create");
    },
    getCreateHelpText : function()
    {
        return qsTr("Creates a new account on the server.");
    },

    primaryPassword : function(input)
    {
        var variables = createAccount.getVariables();
        var passwordVariable = variables.createVariable("password");
        passwordVariable.writeDataString(input);
    },
    confirmPassword : function(input)
    {
        var variables = createAccount.getVariables();
        var confirmPasswordVariable = variables.createVariable("confirmPassword");
        confirmPasswordVariable.writeDataString(input);
    },
    emailAdress : function(input)
    {
        var variables = createAccount.getVariables();
        var emailVariable = variables.createVariable("email");
        emailVariable.writeDataString(input);
    },
    createNewAccount : function()
    {
        var variables = createAccount.getVariables();
        var passwordVariable = variables.createVariable("password");
        var confirmPasswordVariable = variables.createVariable("confirmPassword");
        var emailVariable = variables.createVariable("email");
        var password =  passwordVariable.readDataString();
        var confirmPassword =  confirmPasswordVariable.readDataString();
        var email =  emailVariable.readDataString();
        var menu = createAccount.getBaseMenu();
        if (menu.isValidPassword(password) === false)
        {
            createAccount.showMessageBox(qsTr("Invalid password entered. The password needs to contain 8 letters, one capital one small letter a number and a special char."));
        }
        else if (password !== confirmPassword)
        {
            createAccount.showMessageBox(qsTr("Your passwords don't match."));
        }
        else if (menu.isValidEmailAdress(email) === false)
        {
            createAccount.showMessageBox(qsTr("You entered an invalid email address."));
        }
        else
        {
            CreateAccountDialog.changeEnableForItems(false);
            menu.createServerAccount(password, email);
        }
    },
    changeEnableForItems : function(value)
    {
        createAccount.setObjectEnabled("AbortButton", value);
        createAccount.setObjectEnabled("AccountButton", value);
        createAccount.setObjectEnabled("PasswordBox", value);
        createAccount.setObjectEnabled("ConfirmBox", value);
        createAccount.setObjectEnabled("EmailBox", value);
    },
    onAccountMessage : function(errorCode)
    {
        if (errorCode === GameEnums.LoginError_None)
        {
            createAccount.showMessageBox(qsTr("Account successfully created."));
            createAccount.exit();
        }
        else if (errorCode === GameEnums.LoginError_AccountExists)
        {
            createAccount.showMessageBox(qsTr("An account with the current username exists already. Please change your username in the options menu."));
            CreateAccountDialog.changeEnableForItems(true);
        }
        else
        {
            createAccount.showMessageBox(qsTr("Unknown error happened while creating the new account. No account was created."));
            CreateAccountDialog.changeEnableForItems(true);
        }
    },
};
